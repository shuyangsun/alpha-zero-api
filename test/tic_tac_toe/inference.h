#ifndef ALPHA_ZERO_API_TEST_TIC_TAC_TOE_INFERENCE_H_
#define ALPHA_ZERO_API_TEST_TIC_TAC_TOE_INFERENCE_H_

#include <span>
#include <tuple>
#include <vector>

#include "alpha-zero-api/augmenter.h"
#include "game.h"

namespace alphazero::game::api::test {

class TttInferenceAugmenter
    : public IInferenceAugmenter<TttBoard, TttAction, TttPlayer> {
 public:
  TttInferenceAugmenter() = default;
  ~TttInferenceAugmenter() override = default;

  std::vector<std::tuple<TttBoard, TttPlayer, std::vector<TttAction>>> Augment(
      const TttBoard& board, const TttPlayer& player,
      std::span<const TttAction> actions) const final;

  std::vector<float> Interpret(
      std::span<const std::tuple<TttBoard, TttPlayer, std::vector<TttAction>>>
          augmented_games,
      std::span<const std::vector<float>> outputs) const final;
};

}  // namespace alphazero::game::api::test

#endif  // ALPHA_ZERO_API_TEST_TIC_TAC_TOE_INFERENCE_H_
