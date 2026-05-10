#include "inference.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <span>
#include <unordered_map>
#include <utility>
#include <vector>

#include "alpha-zero-api/policy_output.h"
#include "augmentation.h"
#include "tic_tac_toe/game.h"

namespace az::game::api::test {

namespace {

using ::az::game::api::test::internal::AugmentAll;
using ::az::game::api::test::internal::Augmentation;
using ::az::game::api::test::internal::InverseTransformAction;

}  // namespace

std::vector<TttGame> TttInferenceAugmenter::Augment(
    const TttGame& game) const noexcept {
  return AugmentAll(game);
}

Evaluation TttInferenceAugmenter::Interpret(
    const TttGame& original, std::span<const TttGame> augmented,
    std::span<const Evaluation> evaluations) const noexcept {
  assert(augmented.size() == evaluations.size());
  assert(!augmented.empty());

  std::array<TttAction, TttGame::kMaxLegalActions> orig_actions{};
  const std::size_t orig_count = original.ValidActionsInto(orig_actions);

  std::unordered_map<std::size_t, std::size_t> action_index_map;
  action_index_map.reserve(orig_count);
  for (std::size_t i = 0; i < orig_count; ++i) {
    action_index_map.emplace(original.PolicyIndex(orig_actions[i]), i);
  }

  const float n = static_cast<float>(augmented.size());
  float values_sum = 0.0f;
  std::vector<float> probs(orig_count, 0.0f);

  for (std::size_t i = 0; i < augmented.size(); ++i) {
    const auto sym = static_cast<Augmentation>(i);
    const TttGame& aug_game = augmented[i];
    const Evaluation& eval = evaluations[i];

    values_sum += eval.value;

    std::array<TttAction, TttGame::kMaxLegalActions> aug_actions{};
    const std::size_t aug_count = aug_game.ValidActionsInto(aug_actions);
    assert(aug_count == eval.probabilities.size());

    for (std::size_t j = 0; j < aug_count; ++j) {
      const TttAction original_action =
          InverseTransformAction(aug_actions[j], sym);
      const std::size_t key = original.PolicyIndex(original_action);
      const auto it = action_index_map.find(key);
      assert(it != action_index_map.end());
      probs[it->second] += eval.probabilities[j] / n;
    }
  }

  return Evaluation{values_sum / n, std::move(probs)};
}

}  // namespace az::game::api::test
