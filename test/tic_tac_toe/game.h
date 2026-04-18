#ifndef ALPHA_ZERO_API_TEST_TIC_TAC_TOE_GAME_H_
#define ALPHA_ZERO_API_TEST_TIC_TAC_TOE_GAME_H_

#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "alpha-zero-api/defaults/game.h"

namespace az::game::api::test {

constexpr uint16_t TTT_ROWS = 3;
constexpr uint16_t TTT_COLS = 3;
using TttBoard = Standard2DBoard<TTT_ROWS, TTT_COLS>;
using TttAction = Action2D;
using TttPlayer = BinaryPlayer;
using TttGameInterface = IGame<TttBoard, TttAction, TttPlayer>;

/**
 * @brief An implementation of the Tic Tac Toe game.
 *
 * All methods on this class are const, the game state is immutable. The only
 * way to create a new game state is to call GameAfterAction() with a valid
 * action, which returns an unique pointer to the new game state.
 */
class TttGame : public TttGameInterface {
 public:
  TttGame(TttPlayer starting_player = false);
  TttGame(const TttGame& other) = default;
  TttGame(TttGame&& other) = default;

  ~TttGame() override = default;

  /**
   * @brief Make an identical copy of the current game state.
   *
   * The returned pointer cannot be nullptr, and it points to a new game state
   * that is identical to the current one.
   *
   * @return std::unique_ptr<const TttGameInterface> Unique pointer to the new
   * copy of the game state.
   */
  std::unique_ptr<const TttGameInterface> Copy() const final;

  /**
   * @brief Get the current game board state.
   *
   * @return const TttBoard& Const reference to the current game board state.
   */
  const TttBoard& GetBoard() const final;

  /**
   * @brief Get the current round number.
   *
   * @return uint32_t The current round number.
   */
  uint32_t CurrentRound() const final;

  /**
   * @brief Get the current player.
   *
   * @return TttPlayer The current player.
   */
  TttPlayer CurrentPlayer() const final;

  /**
   * @brief Get the player from last round.
   *
   * @return std::optional<TttPlayer> The player from last round, or
   * std::nullopt if the game has not started yet.
   */
  std::optional<TttPlayer> LastPlayer() const final;

  /**
   * @brief Get the last action taken by the last player.
   *
   * @return std::optional<TttAction> The last action taken by the last player,
   * or std::nullopt if the game has not started yet.
   */
  std::optional<TttAction> LastAction() const final;

  /**
   * @brief The canonical representation of the current board state from the
   * current player's perspective.
   *
   * For example, for Tic Tac Toe, the canonical form of the board can be
   * represented as a 2D array where the current player's pieces are marked with
   * 1, the opponent's pieces are marked with -1, and empty cells are marked
   * with 0. For games with incomplete information (e.g., card games), the
   * canonical form should only include information that is visible to the
   * current player.
   *
   * This helps with training machine learning models by providing a consistent
   * representation of the game state from the perspective of the current
   * player during inference time.
   *
   * If variations from different perspectives are desired to increase variance
   * in the training data, that should be handled by augmenters. More details at
   * https://github.com/shuyangsun/alpha-zero-api/blob/main/src/include/alpha-zero-api/augmenter.h
   *
   * @return TttBoard The canonical representation of the current board state.
   */
  TttBoard CanonicalBoard() const final;

  /**
   * @brief Check if the game is over.
   *
   * If the game is over, no more round can be played by any player. It is safe
   * to call GetScore() after the game is over.
   *
   * @return true If the game is over.
   * @return false If the game is not over.
   */
  bool IsOver() const final;

  /**
   * @brief Get the score of the given player in the current game state.
   *
   * Only guaranteed to return a meaningful score when the game is over. If the
   * game is not over, the returned score may be arbitrary and should not be
   * used for any purpose. The implementation may choose to skip game status
   * check to optimize for performance.
   *
   * @param player The player for which to get the score.
   * @return float The score of the player.
   */
  float GetScore(const TttPlayer& player) const final;

  // --------------------------------- Actions ---------------------------------

  /**
   * @brief Returns a vector of all valid actions for the current player in the
   * current game state.
   *
   * The size of the vector is dynamic, there should be no duplicate actions.
   * This vector should be empty if and only if the game is over. While the game
   * is not over, even if there is no valid action for the current player, the
   * vector should contain at least one action to represent the "pass" action,
   * because the GameAfterAction method requires an action as input.
   *
   * @return std::vector<TttAction> Vector of all valid actions for the current
   * player.
   */
  std::vector<TttAction> ValidActions() const final;

  /**
   * @brief Returns a new game state after the current player takes the given
   * action.
   *
   * The action passed in should be only one of the actions returned by the
   * ValidActions() method. The behavior is undefined if the action is invalid.
   * The implementation may choose to return nullptr after checking the validity
   * of the action, or it may return an invalid game state without paying the
   * performance penalty of validation. Most implementations should choose the
   * latter approach, because the library user is responsible for passing a
   * valid action. However, the library user should program defensively and
   * avoid null pointer dereference.
   *
   * @param action The action to be taken by the current player.
   *
   * @return std::unique_ptr<const TttGameInterface> A pointer to the new game
   * state after taking the action.
   */
  std::unique_ptr<const TttGameInterface> GameAfterAction(
      const TttAction& action) const final;

  // --------------------------- String Conversions ----------------------------

  /**
   * @brief Human-readable string to represent the current board state during
   * gameplay in terminal or for debugging.
   *
   * This is the most basic form of user interface for the game, should be easy
   * to understand for both human and LLM players. LLM agents may choose to run
   * the main binary in the terminal to debug game implementation.
   *
   * @return std::string A human-readable string representing the current board
   * state.
   */
  std::string BoardReadableString() const final;

  /**
   * @brief Convert a human-readable string to an action.
   *
   * May be used by a human player or an LLM agent to play the game in the
   * terminal.
   *
   * @param action_str The string representing the action.
   * @return std::expected<TttAction, std::string> The action if the string is
   * valid, or an error message if the string is invalid.
   */
  std::expected<TttAction, std::string> ActionFromString(
      std::string_view action_str) const final;

  /**
   * @brief Convert an action to a human-readable string.
   *
   * Used to display past actions or available actions to human players or LLM
   * agents in the terminal.
   *
   * @param action The action to be converted to string.
   * @return std::string A human-readable string representing the action.
   */
  std::string ActionToString(const TttAction& action) const final;

 private:
  TttBoard board_ = TttBoard{};
  uint32_t current_round_ = 0;
  TttPlayer current_player_;
  std::optional<TttAction> last_action_ = std::nullopt;
};

}  // namespace az::game::api::test

#endif  // ALPHA_ZERO_API_TEST_TIC_TAC_TOE_GAME_H_
