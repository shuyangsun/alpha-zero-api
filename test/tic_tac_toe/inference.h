#ifndef ALPHA_ZERO_API_TEST_TIC_TAC_TOE_INFERENCE_H_
#define ALPHA_ZERO_API_TEST_TIC_TAC_TOE_INFERENCE_H_

#include <span>
#include <vector>

#include "alpha-zero-api/augmenter.h"
#include "alpha-zero-api/policy_output.h"
#include "tic_tac_toe/game.h"

namespace az::game::api::test {

/**
 * @brief Inference-time augmenter for `TttGame`.
 *
 * `Augment(game)` returns the 12 dihedral variants in the order
 * defined by `internal::Augmentation`; `augmented[0]` is the
 * identity. `Interpret` averages the resulting evaluations after
 * inverting each per-augmentation symmetry to align actions with the
 * original game.
 */
class TttInferenceAugmenter : public IInferenceAugmenter<TttGame> {
 public:
  TttInferenceAugmenter() = default;
  ~TttInferenceAugmenter() override = default;

  [[nodiscard]] std::vector<TttGame> Augment(
      const TttGame& game) const noexcept final;

  [[nodiscard]] Evaluation Interpret(
      const TttGame& original, std::span<const TttGame> augmented,
      std::span<const Evaluation> evaluations) const noexcept final;
};

}  // namespace az::game::api::test

#endif  // ALPHA_ZERO_API_TEST_TIC_TAC_TOE_INFERENCE_H_
