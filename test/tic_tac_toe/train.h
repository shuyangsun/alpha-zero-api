#ifndef ALPHA_ZERO_API_TEST_TIC_TAC_TOE_TRAIN_H_
#define ALPHA_ZERO_API_TEST_TIC_TAC_TOE_TRAIN_H_

#include <span>
#include <tuple>
#include <vector>

#include "alpha-zero-api/augmenter.h"
#include "game.h"

namespace az::game::api::test {

class TttTrainingAugmenter
    : public ITrainingAugmenter<TttBoard, TttAction, TttPlayer> {
 public:
  TttTrainingAugmenter() = default;
  ~TttTrainingAugmenter() override = default;

  [[nodiscard]] std::vector<
      std::tuple<TttBoard, TttPlayer, std::vector<TttAction>, PolicyOutput>>
  Augment(const TttBoard& board, const TttPlayer& player,
          std::span<const TttAction> actions,
          PolicyOutput&& output) const noexcept final;
};

}  // namespace az::game::api::test

#endif  // ALPHA_ZERO_API_TEST_TIC_TAC_TOE_TRAIN_H_
