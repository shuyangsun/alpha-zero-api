#ifndef ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_AUGMENTER_H_
#define ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_AUGMENTER_H_

#include <span>
#include <utility>
#include <vector>

#include "alpha-zero-api/game.h"
#include "alpha-zero-api/policy_output.h"

namespace az::game::api {

/**
 * @brief Inference-time symmetry augmentation.
 *
 * `Augment` returns the set of equivalent positions the engine should
 * evaluate; the engine runs each through the network and feeds the
 * resulting `Evaluation`s back through `Interpret`, which combines
 * them into a single evaluation for the original game.
 *
 * Convention: `augmented[0]` is the identity (`game` itself), so a
 * game with no useful symmetry can return a one-element vector and
 * `Interpret` becomes effectively the identity. Games like chess that
 * have no exploitable board symmetry can supply a no-op augmenter
 * built on this convention.
 */
template <Game G>
class IInferenceAugmenter {
 public:
  virtual ~IInferenceAugmenter() = default;

  [[nodiscard]] virtual std::vector<G> Augment(
      const G& game) const noexcept = 0;

  /**
   * @brief Combine per-augmentation evaluations back into one.
   *
   * `augmented` and `evaluations` are aligned: `evaluations[i]`
   * corresponds to `augmented[i]`. `evaluations[i].probabilities[j]`
   * is the prior for `augmented[i].ValidActions()[j]` — the
   * implementation is responsible for inverting whatever symmetry it
   * applied so the returned `Evaluation`'s probabilities align with
   * `original.ValidActions()`.
   */
  [[nodiscard]] virtual Evaluation Interpret(
      const G& original, std::span<const G> augmented,
      std::span<const Evaluation> evaluations) const noexcept = 0;
};

/**
 * @brief Training-time symmetry augmentation.
 *
 * Returns a vector of `(augmented_game, augmented_target)` pairs
 * derived from `(game, target)`. The augmented target's `pi[i]`
 * corresponds to `augmented_game.ValidActions()[i]`. `target.z` is
 * preserved unchanged (board symmetries are score-preserving).
 *
 * Convention: result includes the identity — typically as the first
 * element, though callers should not rely on order.
 */
template <Game G>
class ITrainingAugmenter {
 public:
  virtual ~ITrainingAugmenter() = default;

  [[nodiscard]] virtual std::vector<std::pair<G, TrainingTarget>> Augment(
      const G& game, const TrainingTarget& target) const noexcept = 0;
};

}  // namespace az::game::api

#endif  // ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_AUGMENTER_H_
