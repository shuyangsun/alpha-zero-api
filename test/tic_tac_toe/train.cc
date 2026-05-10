#include "train.h"

#include <array>
#include <cassert>
#include <cstddef>
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

std::vector<std::pair<TttGame, TrainingTarget>> TttTrainingAugmenter::Augment(
    const TttGame& game, const TrainingTarget& target) const noexcept {
  std::vector<TttGame> augmented = AugmentAll(game);
  std::vector<std::pair<TttGame, TrainingTarget>> result;
  result.reserve(augmented.size());

  std::array<TttAction, TttGame::kMaxLegalActions> orig_actions{};
  const std::size_t orig_count = game.ValidActionsInto(orig_actions);
  assert(target.pi.size() == orig_count);

  std::unordered_map<std::size_t, std::size_t> orig_index;
  orig_index.reserve(orig_count);
  for (std::size_t i = 0; i < orig_count; ++i) {
    orig_index.emplace(game.PolicyIndex(orig_actions[i]), i);
  }

  for (std::size_t i = 0; i < augmented.size(); ++i) {
    const auto sym = static_cast<Augmentation>(i);
    TttGame& aug_game = augmented[i];
    std::array<TttAction, TttGame::kMaxLegalActions> aug_actions{};
    const std::size_t aug_count = aug_game.ValidActionsInto(aug_actions);

    std::vector<float> aug_pi(aug_count, 0.0f);
    for (std::size_t j = 0; j < aug_count; ++j) {
      const TttAction original_action =
          InverseTransformAction(aug_actions[j], sym);
      const std::size_t key = game.PolicyIndex(original_action);
      const auto it = orig_index.find(key);
      assert(it != orig_index.end());
      aug_pi[j] = target.pi[it->second];
    }

    result.emplace_back(std::move(aug_game),
                        TrainingTarget{target.z, std::move(aug_pi)});
  }

  return result;
}

}  // namespace az::game::api::test
