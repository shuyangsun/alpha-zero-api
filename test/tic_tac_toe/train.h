#ifndef ALPHA_ZERO_API_TEST_TIC_TAC_TOE_TRAIN_H_
#define ALPHA_ZERO_API_TEST_TIC_TAC_TOE_TRAIN_H_

#include <utility>
#include <vector>

#include "alpha-zero-api/augmenter.h"
#include "alpha-zero-api/policy_output.h"
#include "tic_tac_toe/game.h"

namespace az::game::api::test {

/**
 * @brief Training-time augmenter for `TttGame`.
 *
 * Returns 12 `(augmented_game, augmented_target)` pairs in the order
 * given by `internal::Augmentation`. Each pair's `pi[i]` corresponds
 * to `augmented_game.ValidActions()[i]`; `target.z` is preserved
 * unchanged.
 */
class TttTrainingAugmenter : public ITrainingAugmenter<TttGame> {
 public:
  TttTrainingAugmenter() = default;
  ~TttTrainingAugmenter() override = default;

  [[nodiscard]] std::vector<std::pair<TttGame, TrainingTarget>> Augment(
      const TttGame& game, const TrainingTarget& target) const noexcept final;
};

}  // namespace az::game::api::test

#endif  // ALPHA_ZERO_API_TEST_TIC_TAC_TOE_TRAIN_H_
