#ifndef ALPHA_ZERO_API_API_CPP_AUGMENTER_H_
#define ALPHA_ZERO_API_API_CPP_AUGMENTER_H_

#include <memory>
#include <utility>
#include <vector>

#include "api/cpp/game.h"
#include "api/cpp/policy_output.h"

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
   * @brief Augment the game for inference.
   *
   * This is usually used to minimize the bias of the neural network. In the
   * original AlphaGo Zero paper, each position is augmented 8 times. The order
   * of returned augmented games should be deterministic, so it can be properly
   * interpreted later.
   *
   * @param game Game to augment for inference.
   * @return std::vector<std::shared_ptr<const IGame<B, A, P>>> Vector of
   * augmented games.
   */
  virtual std::vector<std::shared_ptr<const IGame<B, A, P>>>
  AugmentInferenceData(const IGame<B, A, P>& game) const = 0;

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
  virtual std::vector<
      std::pair<std::shared_ptr<const IGame<B, A, P>>, PolicyOutput>>
  AugmentTrainingData(const IGame<B, A, P>& game,
                      PolicyOutput&& output) const = 0;

  virtual ~IAugmenter() = default;
};

}  // namespace alphazero::game::api

#endif  // ALPHA_ZERO_API_API_CPP_AUGMENTER_H_
