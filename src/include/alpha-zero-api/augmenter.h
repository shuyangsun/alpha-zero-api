#ifndef ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_AUGMENTER_H_
#define ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_AUGMENTER_H_

#include <cstdint>
#include <memory>
#include <span>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "alpha-zero-api/game.h"
#include "alpha-zero-api/policy_output.h"

namespace az::game::api {

/**
 * @brief Augment the game before inference to reduce model bias.
 *
 * @tparam B Type of board. See documentation for IGame in api/cpp/game.h.
 * @tparam A Type of a single action. See documentation for IGame in
 * api/cpp/game.h.
 * @tparam P Type of player. See documentation for IGame in api/cpp/game.h.
 */
template <typename B, typename A, typename P>
class IInferenceAugmenter {
 public:
  /**
   * @brief Augment the game state for inference.
   *
   * The function parameters are the same as Serializer<B, A, P>::Serialize.
   * This is usually used to minimize the bias of the neural network. In the
   * original AlphaGo Zero paper, each position is augmented 8 times.
   *
   * @param board Current game board.
   * @param player Current player.
   * @param actions Valid actions for current player.
   * @return std::unordered_map<uint8_t, std::tuple<B, P, std::vector<A>>> Map
   * of augmented game state. The key is an identifier for the augmented game
   * state, and the value is a tuple of the augmented board, player, and
   * actions.
   */
  virtual std::unordered_map<uint8_t, std::tuple<B, P, std::vector<A>>> Augment(
      const B& board, const P& player, std::span<const A> actions) const = 0;

  /**
   * @brief Interpret the inference-time policy output from different augmented
   * games. The order of the outputs corresponds to the order of augmented
   * games.
   *
   * @param augmented_games The augmented game states that were generated from
   * the Augment function.
   * @param outputs The policy outputs for each augmented game. The key is the
   * identifier for the augmented game, and the value is the policy output for
   * that augmented game.
   * @return PolicyOutput Interpreted policy output for the original game.
   */
  virtual PolicyOutput Interpret(
      const std::unordered_map<uint8_t, std::tuple<B, P, std::vector<A>>>&
          augmented_games,
      const std::unordered_map<uint8_t, PolicyOutput>& outputs) const = 0;

  virtual ~IInferenceAugmenter() = default;
};

/**
 * @brief Augment the game state and policy output for training.
 *
 * @tparam B Type of board. See documentation for IGame in api/cpp/game.h.
 * @tparam A Type of a single action. See documentation for IGame in
 * api/cpp/game.h.
 * @tparam P Type of player. See documentation for IGame in api/cpp/game.h.
 */
template <typename B, typename A, typename P>
class ITrainingAugmenter {
 public:
  /**
   * @brief Augment the game state and policy output for training.
   *
   * @param board Current game board.
   * @param player Current player.
   * @param actions Valid actions for current player.
   * @param output Policy output for the current game state.
   * @return std::vector<
   * std::pair<std::shared_ptr<const IGame<B, A, P>>, PolicyOutput>> Vector of
   * augmented game and policy output pairs.
   */
  virtual std::vector<std::tuple<B, P, std::vector<A>, PolicyOutput>> Augment(
      const B& board, const P& player, std::span<const A> actions,
      PolicyOutput&& output) const = 0;

  virtual ~ITrainingAugmenter() = default;
};

}  // namespace az::game::api

#endif  // ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_AUGMENTER_H_
