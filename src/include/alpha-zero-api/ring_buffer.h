#ifndef ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_RING_BUFFER_H_
#define ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_RING_BUFFER_H_

#include <compare>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

namespace az::game::api {

/**
 * @brief Concept describing the underlying storage backend for `RingBuffer`.
 *
 * A conforming storage type must:
 *   - Expose a `value_type` member type.
 *   - Be indexable with `operator[](std::size_t)` returning a mutable
 *     reference to `value_type`.
 *   - Provide `size()` returning a value convertible to `std::size_t`.
 *   - Provide `data()` returning a pointer convertible to
 *     `const value_type*` (i.e. the storage is contiguous).
 *
 * `std::array<T, N>` and `std::vector<T>` are the two canonical storage
 * types that satisfy this concept, providing stack-allocated and
 * heap-allocated rings respectively.
 *
 * @tparam S The storage type to constrain.
 */
template <typename S>
concept RingBufferStorage = requires(S s, std::size_t i) {
  typename S::value_type;
  { s[i] } -> std::same_as<typename S::value_type&>;
  { s.size() } -> std::convertible_to<std::size_t>;
  { s.data() } -> std::convertible_to<const typename S::value_type*>;
};

template <typename T>
class RingBufferView;

/**
 * @brief Fixed-capacity ring buffer parameterized on its storage backend.
 *
 * `RingBuffer` retains the most recent `Capacity()` elements pushed into
 * it. `Push` is O(1) and never allocates on the hot path; once the
 * buffer is full, the next `Push` overwrites the oldest element in
 * place. The buffer is intended for use as engine-owned game-state
 * history feeding consumers (serializers, augmenters) that need a
 * recent window of past positions.
 *
 * Two canonical instantiations:
 *   - `RingBuffer<T, std::array<T, N>>` — stack-allocated, capacity
 *     fixed at compile time. Constructed via the default constructor.
 *   - `RingBuffer<T, std::vector<T>>` — heap-allocated, capacity fixed
 *     at construction time. Constructed via the explicit-capacity
 *     constructor.
 *
 * NOTE: a buffer with `Capacity() == 0` is well-formed; `Push` is a
 * no-op and `View()` returns an empty view. This is the natural state
 * for game types declaring zero history lookback.
 *
 * @tparam T The element type. Must match `Storage::value_type`.
 * @tparam Storage The contiguous, indexable backing storage. Must
 *   satisfy `RingBufferStorage` and have `value_type == T`.
 */
template <typename T, RingBufferStorage Storage>
  requires std::same_as<typename Storage::value_type, T>
class RingBuffer {
 public:
  /**
   * @brief Default constructor.
   *
   * Suitable for storage types whose capacity is set at compile time
   * (e.g. `std::array<T, N>`). For runtime-sized storage like
   * `std::vector<T>`, the resulting buffer has zero capacity; use the
   * explicit-capacity constructor instead.
   */
  RingBuffer() = default;

  /**
   * @brief Construct a ring buffer with a runtime-specified capacity.
   *
   * Available only when `Storage` is constructible from a single
   * `std::size_t` argument (e.g. `std::vector<T>`).
   *
   * @param capacity Maximum number of elements the buffer will retain.
   */
  explicit RingBuffer(std::size_t capacity) noexcept(
      std::is_nothrow_constructible_v<Storage, std::size_t>)
    requires std::constructible_from<Storage, std::size_t>
      : storage_(capacity) {}

  /**
   * @brief Push a new element onto the buffer (copy overload).
   *
   * If the buffer is at capacity, the oldest element is overwritten.
   * If `Capacity() == 0`, this call is a no-op.
   *
   * @param value Element to insert.
   */
  void Push(const T& value) noexcept {
    const std::size_t cap = storage_.size();
    if (cap == 0) return;
    storage_[head_] = value;
    head_ = (head_ + 1) % cap;
    if (size_ < cap) ++size_;
  }

  /**
   * @brief Push a new element onto the buffer (move overload).
   *
   * If the buffer is at capacity, the oldest element is overwritten.
   * If `Capacity() == 0`, this call is a no-op.
   *
   * @param value Element to insert.
   */
  void Push(T&& value) noexcept {
    const std::size_t cap = storage_.size();
    if (cap == 0) return;
    storage_[head_] = std::move(value);
    head_ = (head_ + 1) % cap;
    if (size_ < cap) ++size_;
  }

  /**
   * @brief Number of elements currently retained. Always `<= Capacity()`.
   */
  [[nodiscard]] std::size_t Size() const noexcept { return size_; }

  /**
   * @brief Maximum number of elements the buffer will retain.
   */
  [[nodiscard]] std::size_t Capacity() const noexcept {
    return storage_.size();
  }

  /**
   * @brief True iff the buffer holds no elements.
   */
  [[nodiscard]] bool Empty() const noexcept { return size_ == 0; }

  /**
   * @brief Read-only window over the buffer's current contents.
   *
   * The returned view iterates **newest-first**: index 0 is the most
   * recently pushed element, index `Size() - 1` is the oldest still in
   * the window. The view is a value-semantic snapshot — any subsequent
   * `Push` invalidates it (the underlying storage pointer remains
   * valid for the lifetime of the buffer, but the view's recorded
   * head and size go stale).
   */
  [[nodiscard]] RingBufferView<T> View() const noexcept {
    const std::size_t cap = storage_.size();
    if (size_ == 0 || cap == 0) {
      return RingBufferView<T>{};
    }
    const std::size_t newest = (head_ + cap - 1) % cap;
    return RingBufferView<T>{storage_.data(), cap, newest, size_};
  }

 private:
  Storage storage_{};
  std::size_t head_ = 0;
  std::size_t size_ = 0;
};

/**
 * @brief Read-only, newest-first window over a `RingBuffer`'s contents.
 *
 * `RingBufferView<T>` is a cheap value-semantic snapshot: it stores a
 * pointer into the source buffer's storage plus three `std::size_t`s,
 * and exposes a random-access iterator interface so the view composes
 * with range-based `for` and `std::ranges` algorithms.
 *
 * Indexing is newest-first: `view[0]` is the most recently pushed
 * element, `view[Size() - 1]` is the oldest element still retained.
 *
 * Views are invalidated by any subsequent `Push` on the source buffer.
 *
 * @tparam T The element type.
 */
template <typename T>
class RingBufferView {
 public:
  /**
   * @brief Random-access iterator over a `RingBufferView`'s contents.
   *
   * The iterator walks the view newest-first: `*begin()` is the
   * newest element, `*(end() - 1)` is the oldest in the window.
   */
  class const_iterator {
   public:
    using iterator_concept = std::random_access_iterator_tag;
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = const T*;
    using reference = const T&;

    const_iterator() noexcept = default;

    [[nodiscard]] reference operator*() const noexcept {
      return data_[ResolvedIndex()];
    }

    [[nodiscard]] reference operator[](difference_type n) const noexcept {
      return *(*this + n);
    }

    const_iterator& operator++() noexcept {
      ++offset_;
      return *this;
    }
    const_iterator operator++(int) noexcept {
      const_iterator tmp = *this;
      ++offset_;
      return tmp;
    }
    const_iterator& operator--() noexcept {
      --offset_;
      return *this;
    }
    const_iterator operator--(int) noexcept {
      const_iterator tmp = *this;
      --offset_;
      return tmp;
    }

    const_iterator& operator+=(difference_type n) noexcept {
      offset_ += n;
      return *this;
    }
    const_iterator& operator-=(difference_type n) noexcept {
      offset_ -= n;
      return *this;
    }

    [[nodiscard]] friend const_iterator operator+(const_iterator it,
                                                  difference_type n) noexcept {
      it += n;
      return it;
    }
    [[nodiscard]] friend const_iterator operator+(difference_type n,
                                                  const_iterator it) noexcept {
      it += n;
      return it;
    }
    [[nodiscard]] friend const_iterator operator-(const_iterator it,
                                                  difference_type n) noexcept {
      it -= n;
      return it;
    }
    [[nodiscard]] friend difference_type operator-(
        const const_iterator& a, const const_iterator& b) noexcept {
      return a.offset_ - b.offset_;
    }

    [[nodiscard]] friend bool operator==(const const_iterator& a,
                                         const const_iterator& b) noexcept {
      return a.offset_ == b.offset_;
    }
    [[nodiscard]] friend std::strong_ordering operator<=>(
        const const_iterator& a, const const_iterator& b) noexcept {
      return a.offset_ <=> b.offset_;
    }

   private:
    friend class RingBufferView<T>;

    const_iterator(const T* data, std::size_t capacity,
                   std::size_t newest_index, difference_type offset) noexcept
        : data_{data},
          capacity_{capacity},
          newest_index_{newest_index},
          offset_{offset} {}

    [[nodiscard]] std::size_t ResolvedIndex() const noexcept {
      const auto cap = static_cast<difference_type>(capacity_);
      auto idx = static_cast<difference_type>(newest_index_) - offset_;
      idx = ((idx % cap) + cap) % cap;
      return static_cast<std::size_t>(idx);
    }

    const T* data_ = nullptr;
    std::size_t capacity_ = 0;
    std::size_t newest_index_ = 0;
    difference_type offset_ = 0;
  };

  RingBufferView() noexcept = default;

  /**
   * @brief Construct a view directly. Intended for use by `RingBuffer`;
   * end users should obtain views via `RingBuffer::View()`.
   *
   * @param data Pointer to the source buffer's contiguous storage.
   * @param capacity Capacity of the source buffer (modulus for ring
   *   arithmetic; must be `> 0` if `size > 0`).
   * @param newest_index Index of the newest element in the source
   *   storage.
   * @param size Number of elements in the window; must be `<= capacity`.
   */
  RingBufferView(const T* data, std::size_t capacity, std::size_t newest_index,
                 std::size_t size) noexcept
      : data_{data},
        capacity_{capacity},
        newest_index_{newest_index},
        size_{size} {}

  /**
   * @brief Number of elements visible through the view.
   */
  [[nodiscard]] std::size_t Size() const noexcept { return size_; }

  /**
   * @brief True iff the view is empty.
   */
  [[nodiscard]] bool Empty() const noexcept { return size_ == 0; }

  /**
   * @brief Newest-first index into the view. `i` must be `< Size()`.
   */
  [[nodiscard]] const T& operator[](std::size_t i) const noexcept {
    return data_[(newest_index_ + capacity_ - i) % capacity_];
  }

  [[nodiscard]] const_iterator begin() const noexcept {
    return const_iterator{data_, capacity_, newest_index_, 0};
  }

  [[nodiscard]] const_iterator end() const noexcept {
    return const_iterator{
        data_, capacity_, newest_index_,
        static_cast<typename const_iterator::difference_type>(size_)};
  }

 private:
  const T* data_ = nullptr;
  std::size_t capacity_ = 0;
  std::size_t newest_index_ = 0;
  std::size_t size_ = 0;
};

}  // namespace az::game::api

#endif  // ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_RING_BUFFER_H_
