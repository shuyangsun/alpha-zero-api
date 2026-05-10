#ifndef ALPHA_ZERO_API_TEST_TIC_TAC_TOE_GAME_H_
#define ALPHA_ZERO_API_TEST_TIC_TAC_TOE_GAME_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <string_view>

#include "alpha-zero-api/defaults/game.h"
#include "alpha-zero-api/game.h"

namespace az::game::api::test {

constexpr uint16_t TTT_ROWS = 3;
constexpr uint16_t TTT_COLS = 3;
constexpr std::size_t TTT_CELLS = static_cast<std::size_t>(TTT_ROWS) * TTT_COLS;

using TttBoard = Standard2DBoard<TTT_ROWS, TTT_COLS>;
using TttAction = Action2D;
using TttPlayer = BinaryPlayer;

/**
 * @brief Error type for TttGame failure.
 */
enum class TttError : uint8_t {
  kUnknownError = 0,
  kNotImplemented,
  kInvalidActionFormat,
  kInvalidActionColumnType,
  kInvalidActionColumnRange,
  kInvalidActionRowType,
  kInvalidActionRowRange,
  kInvalidPolicyOutputSize,
};

template <typename T>
using TttResult = std::expected<T, TttError>;

using TttStatus = TttResult<void>;

/**
 * @brief A value-typed implementation of Tic-Tac-Toe satisfying the
 * `az::game::api::Game` concept.
 *
 * The game is plain data: a 3x3 board, a current-player flag, a round
 * counter, and a fixed-capacity action history used for
 * `UndoLastAction`. Copies are trivial and stack-friendly. The MCTS
 * hot loop is expected to drive the game via `ApplyActionInPlace` /
 * `UndoLastAction` on a single working copy.
 */
class TttGame {
 public:
  using board_t = TttBoard;
  using action_t = TttAction;
  using player_t = TttPlayer;
  using error_t = TttError;

  // Tic-Tac-Toe is Markov: the network sees only the current board.
  static constexpr std::size_t kHistoryLookback = 0;

  // The action space is the 9 cells of the board.
  static constexpr std::size_t kPolicySize = TTT_CELLS;

  // TTT is a dense game: every empty cell is a legal action, so the
  // legal-action ceiling matches the full action space.
  static constexpr std::size_t kMaxLegalActions = TTT_CELLS;

  // The game can never run longer than 9 plies.
  static constexpr std::optional<uint32_t> kMaxRounds =
      static_cast<uint32_t>(TTT_CELLS);

  /**
   * @brief Construct an empty game with `Player1` to move.
   */
  TttGame() noexcept = default;

  /**
   * @brief Construct an empty game with the given starting player.
   */
  explicit TttGame(TttPlayer starting_player) noexcept;

  /**
   * @brief Construct a game from an arbitrary snapshot.
   *
   * Used by augmenters to materialize a transformed position. The
   * resulting game's action history is a single-step trail derived
   * from `last_action`, which is enough for one undo but not for
   * arbitrary backtracking. Augmented games are not intended to be
   * driven by MCTS undo loops.
   */
  TttGame(TttBoard board, TttPlayer current_player, uint32_t current_round,
          std::optional<TttAction> last_action) noexcept;

  TttGame(const TttGame&) noexcept = default;
  TttGame(TttGame&&) noexcept = default;
  TttGame& operator=(const TttGame&) noexcept = default;
  TttGame& operator=(TttGame&&) noexcept = default;
  ~TttGame() = default;

  // ------------------------------- Observers --------------------------------

  [[nodiscard]] const TttBoard& GetBoard() const noexcept;
  [[nodiscard]] uint32_t CurrentRound() const noexcept;
  [[nodiscard]] TttPlayer CurrentPlayer() const noexcept;
  [[nodiscard]] std::optional<TttPlayer> LastPlayer() const noexcept;
  [[nodiscard]] std::optional<TttAction> LastAction() const noexcept;
  [[nodiscard]] TttBoard CanonicalBoard() const noexcept;
  [[nodiscard]] std::size_t ValidActionsInto(
      std::array<TttAction, TTT_CELLS>& out) const noexcept;
  [[nodiscard]] bool IsOver() const noexcept;
  [[nodiscard]] float GetScore(const TttPlayer& player) const noexcept;

  /**
   * @brief Map an action onto its slot in the fixed-size policy output.
   *
   * The mapping is `row * TTT_COLS + col`, deterministic and stable
   * across calls. Used by the default policy serializer/deserializer
   * to lay out the network's policy head.
   */
  [[nodiscard]] std::size_t PolicyIndex(const TttAction& action) const noexcept;

  // ------------------------------- Mutation ---------------------------------

  /**
   * @brief Apply `action` for the current player in place.
   *
   * Caller is responsible for passing a valid action (one of those
   * returned by `ValidActionsInto`). No validation is performed; the
   * MCTS hot loop must not pay for it.
   */
  void ApplyActionInPlace(const TttAction& action) noexcept;

  /**
   * @brief Undo the most recent `ApplyActionInPlace`.
   *
   * Pops the trailing action from the internal history. A no-op if
   * the history is empty.
   */
  void UndoLastAction() noexcept;

  // --------------------------- String Conversions ----------------------------

  [[nodiscard]] std::string BoardReadableString() const noexcept;
  [[nodiscard]] std::expected<TttAction, TttError> ActionFromString(
      std::string_view action_str) const noexcept;
  [[nodiscard]] std::string ActionToString(
      const TttAction& action) const noexcept;

 private:
  TttBoard board_{};
  uint32_t current_round_ = 0;
  TttPlayer current_player_ = false;
  std::array<TttAction, TTT_CELLS> action_history_{};
  uint8_t history_size_ = 0;
};

}  // namespace az::game::api::test

#endif  // ALPHA_ZERO_API_TEST_TIC_TAC_TOE_GAME_H_
