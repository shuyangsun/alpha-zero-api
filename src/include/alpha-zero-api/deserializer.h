#ifndef ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DESERIALIZER_H_
#define ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DESERIALIZER_H_

#include <expected>
#include <span>

#include "alpha-zero-api/game.h"
#include "alpha-zero-api/policy_output.h"

namespace az::game::api {

/**
 * @brief Decode a network forward-pass into an `Evaluation`.
 *
 * Implementations gather the masked subset of policy logits/probs
 * corresponding to `game.ValidActionsInto(...)` (typically via
 * `game.PolicyIndex(a)`) and produce probabilities sized to the returned
 * legal-action count.
 *
 * The signature takes `std::span<const float>`, so callers must
 * up-convert FP16/BF16 outputs to FP32 before invoking the
 * deserializer. Half-precision support, if needed, belongs in a
 * dedicated overload taking `std::span<const std::byte>` plus a
 * precision tag.
 *
 * @tparam G Concrete game type satisfying the `Game` concept.
 * @tparam E Error type returned on malformed input. Implementations
 *   typically use the game's own `error_t`.
 */
template <Game G, typename E>
class IPolicyOutputDeserializer {
 public:
  virtual ~IPolicyOutputDeserializer() = default;

  [[nodiscard]] virtual std::expected<Evaluation, E> Deserialize(
      const G& game, std::span<const float> output) const noexcept = 0;
};

/**
 * @brief Compact policy-output deserializer.
 *
 * Reads a `CompactPolicyOutputBlob` (one value scalar + a row of
 * length `legal_indices.size()`) and produces an `Evaluation` whose
 * `probabilities[i]` is the prior for the i-th action written by
 * `game.ValidActionsInto(...)`. The deserializer is responsible for
 * reordering from `legal_indices` order into `ValidActionsInto` order via
 * `PolicyIndex`.
 *
 * Compact heads typically emit logits, so the canonical implementation
 * applies softmax over `values`. Document explicitly which side
 * normalizes — like the dense interface, it must match between
 * serializer and deserializer.
 *
 * @tparam G Concrete game type satisfying the `Game` concept.
 * @tparam E Error type returned on malformed input. Implementations
 *   typically use the game's own `error_t`.
 */
template <Game G, typename E>
class ICompactPolicyOutputDeserializer {
 public:
  virtual ~ICompactPolicyOutputDeserializer() = default;

  [[nodiscard]] virtual std::expected<Evaluation, E> Deserialize(
      const G& game,
      const CompactPolicyOutputBlob& output) const noexcept = 0;
};

}  // namespace az::game::api

#endif  // ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DESERIALIZER_H_
