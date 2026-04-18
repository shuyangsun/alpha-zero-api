#ifndef ALPHA_ZERO_API_TEST_TIC_TAC_TOE_INFERENCE_H_
#define ALPHA_ZERO_API_TEST_TIC_TAC_TOE_INFERENCE_H_

#include <cstdint>
#include <span>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "alpha-zero-api/augmenter.h"
#include "alpha-zero-api/policy_output.h"
#include "game.h"

namespace az::game::api::test {

class TttInferenceAugmenter
    : public IInferenceAugmenter<TttBoard, TttAction, TttPlayer> {
 public:
  TttInferenceAugmenter() = default;
  ~TttInferenceAugmenter() override = default;

  std::unordered_map<uint8_t,
                     std::tuple<TttBoard, TttPlayer, std::vector<TttAction>>>
  Augment(const TttBoard& board, const TttPlayer& player,
          std::span<const TttAction> actions) const final;

  PolicyOutput Interpret(
      const std::unordered_map<
          uint8_t, std::tuple<TttBoard, TttPlayer, std::vector<TttAction>>>&
          augmented_games,
      const std::unordered_map<uint8_t, PolicyOutput>& outputs) const final;
};

}  // namespace az::game::api::test

#endif  // ALPHA_ZERO_API_TEST_TIC_TAC_TOE_INFERENCE_H_
