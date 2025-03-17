#ifndef ALPHA_ZERO_API_API_CPP_GAME_H_
#define ALPHA_ZERO_API_API_CPP_GAME_H_

#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace alphazero::game::api {

/**
 * @brief IGame is the interface of a specific game implementation. It is the
 * most important and complex interface out of all components.
 *
 * @tparam B Type of game board. This is usually an array or vector for most
 * board games.
 * @tparam A Type of a single action a player can take given a game state. For
 * example, in the game TicTacToe, this can be integer representing the position
 * on the board.
 * @tparam P Type of player. For two player games, this is usually bool; for
 * multi-player games, this is usually unsigned int.
 */
template <typename B, typename A, typename P>
class IGame {
 public:
  /**
   * @brief Copy the current game state.
   *
   * @return std::unique_ptr<const IGame<B, A, P>> Pointer to a new copy of this
   * game object.
   */
  virtual std::unique_ptr<const IGame<B, A, P>> Copy() const = 0;

  /**
   * @brief Get current board state.
   *
   * @return const B& Reference to the current game board state.
   */
  virtual const B& GetBoard() const = 0;

  /**
   * @brief Get current round (turn) number.
   *
   * @return uint32_t Current round number.
   */
  virtual uint32_t CurrentRound() const = 0;

  /**
   * @brief Get current player, the player that needs to take an action.
   *
   * @return P Player that will take the next action.
   */
  virtual P CurrentPlayer() const = 0;

  /**
   * @brief Get the player that played the round before the current round.
   *
   * Usually, if the game has not started yet, this function should return
   * std::nullopt. After the game starts, this function should return the player
   * that took the last action.
   *
   * @return std::optional<P> Player that took the last action.
   */
  virtual std::optional<P> LastPlayer() const = 0;

  /**
   * @brief Get the last action last player took, which lead to the current
   * game state.
   *
   * Usually, if the game has not started yet, this function should return
   * std::nullopt. After the game starts, this function should return the action
   * taken by the last player.
   *
   * NOTE: If the game mechnism allows a player to pass a round, this method
   * should still return something to represent the "pass" action. Similarly, if
   * the game allows players to undo an action, this method should return
   * something to represent the "undo" action. From the library's perspective,
   * the game state can only proceed by taking an action, even if the action is
   * "pass" or "undo".
   *
   * @return std::optional<A> The action taken by the last player.
   */
  virtual std::optional<A> LastAction() const = 0;

  /**
   * @brief Get the canonical form of the current board state.
   *
   * The canonical form is the form of the board that is invariant to the
   * current player, meaning that the board should be from the current player's
   * perspective.
   *
   * @return B Canonical form of the current board state from the current
   * player's perspective.
   */
  virtual B CanonicalBoard() const = 0;

  /**
   * @brief Vector of all valid actions the current player can take.
   *
   * This function should return all possible actions the current player can
   * take. The returned vector can be empty if the current player has no valid
   * action. The returned vector can vary in length depending on the current
   * game state.
   *
   * @return std::vector<A> Actions the current player can take.
   */
  virtual std::vector<A> Actions() const = 0;

  /**
   * @brief Game state after the current player takes a given action.
   *
   * This function should return a new game object after the current player
   * takes the given action.
   *
   * NOTE: all values returned from the `Actions()` method can be used as input
   * to this method. It is the responsibility of the game implementation to
   * ensure that all actions are valid. The library is free to pass any value
   * from the result of the `Actions()` method to this method, and the library
   * guarantees to only pass values from valid actions.
   *
   * @param action Action to take from the current game state by the current
   * player.
   * @return std::unique_ptr<const IGame> Pointer to a new game object after the
   * action is taken by the current player.
   */
  virtual std::unique_ptr<const IGame> GameAfterAction(
      const A& action) const = 0;

  /**
   * @brief Check if the game ended.
   *
   * Game is over usually means a player winning or losing, or the game is a
   * draw. It is up to the game implementation to define what is considered as
   * over.
   *
   * @return true Game is over.
   * @return false Game is not over.
   */
  virtual bool IsOver() const = 0;

  /**
   * @brief Get the score of given player when the game is over.
   *
   * The library guarantees that this method is only called when the game is
   * already over. For example, if the game is binary-player and binary-outcome,
   * the score is usually 1 for win, 0 for draw, and -1 for lose.
   *
   * @param player Player to get the score.
   * @return float Score of the given player after the game ends.
   */
  virtual float GetScore(const P& player) const = 0;

  /**
   * @brief Print the current game board in a human-readable format.
   *
   * This is used to play with the game in the console. The output should be
   * human-readable and easy to understand.
   *
   * @return std::string Readable string of the current game board.
   */
  virtual std::string BoardReadableString() const = 0;

  /**
   * @brief Action from a human-readable string.
   *
   * This is used to play the game in the console, converts user input to an
   * action.
   *
   * @param action_str User input of the next action they want to take.
   * @return std::expected<A, std::string>
   */
  virtual std::expected<A, std::string> ActionFromString(
      std::string_view action_str) const = 0;

  /**
   * @brief Converts an action to a human-readable string.
   *
   * This is used to play the game in the console, prints an action from a
   * player in a readable format.
   *
   * @param action Action to convert to a readable string.
   * @return std::string Readable string of the action.
   */
  virtual std::string ActionToString(const A& action) const = 0;

  virtual ~IGame() = default;
};

}  // namespace alphazero::game::api

#endif  // ALPHA_ZERO_API_API_CPP_GAME_H_
