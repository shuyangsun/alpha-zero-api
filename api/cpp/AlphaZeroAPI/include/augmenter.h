#ifndef ALPHA_ZERO_API_API_CPP_ALPHAZEROAPI_INCLUDE_AUGMENTER_H_
#define ALPHA_ZERO_API_API_CPP_ALPHAZEROAPI_INCLUDE_AUGMENTER_H_

#include <memory>
#include <span>
#include <tuple>
#include <vector>

#include "AlphaZeroAPI/include/game.h"
#include "AlphaZeroAPI/include/policy_output.h"

namespace alphazero::game::api {

/**
 * @brief Augment the game before inference or sample data for training.
 *
 * @tparam B Type of board. See documentation for IGame in api/cpp/game.h.
 * @tparam A Type of a single action. See documentation for IGame in
 * api/cpp/game.h.
 * @tparam P Type of player. See documentation for IGame in api/cpp/game.h.
 */
template <typename B, typename A, typename P>
class IAugmenter {
 public:
  /**
   * @brief Augment the game state for inference.
   *
   * The function parameters are the same as Serializer<B, A, P>::Serialize.
   * This is usually used to minimize the bias of the neural network. In the
   * original AlphaGo Zero paper, each position is augmented 8 times. The order
   * of returned augmented games should be deterministic, so it can be properly
   * interpreted later.
   *
   * @param board Current game board.
   * @param player Current player.
   * @param actions Valid actions for current player.
   * @return std::vector<std::tuple<B, P, std::vector<A>>> Vector of augmented
   * board, player and valid actions.
   */
  virtual std::vector<std::tuple<B, P, std::vector<A>>> AugmentInferenceData(
      const B& board, const P& player, std::span<const A> actions) const = 0;

  /**
   * @brief Interpret the inference-time policy output from different augmented
   * games. The order of the outputs corresponds to the order of augmented
   * games.
   *
   * @param outputs Policy outputs from the neural network for each augmented
   * game.
   * @return PolicyOutput Consolidated policy output from all outputs.
   */
  virtual PolicyOutput InterpretPolicyOutputs(
      std::span<const PolicyOutput> outputs) const = 0;

  /**
   * @brief Augment the game state and policy output for training.
   *
   * TODO: change to augment board, player and actions.
   *
   * @param game
   * @param output
   * @return std::vector<
   * std::pair<std::shared_ptr<const IGame<B, A, P>>, PolicyOutput>>
   */
  virtual std::vector<std::tuple<B, P, std::vector<A>, PolicyOutput>>
  AugmentTrainingData(const B& board, const P& player,
                      std::span<const A> actions,
                      PolicyOutput&& output) const = 0;

  virtual ~IAugmenter() = default;
};

}  // namespace alphazero::game::api

#endif  // ALPHA_ZERO_API_API_CPP_ALPHAZEROAPI_INCLUDE_AUGMENTER_H_
