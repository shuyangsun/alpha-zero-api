#include "train.h"

#include <cstdint>
#include <span>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "augmentation.h"
#include "game.h"

namespace alphazero::game::api::test {

std::vector<
    std::tuple<TttBoard, TttPlayer, std::vector<TttAction>, PolicyOutput>>
TttTrainingAugmenter::Augment(const TttBoard& board, const TttPlayer& player,
                              std::span<const TttAction> actions,
                              PolicyOutput&& output) const {
  std::unordered_map<uint8_t,
                     std::tuple<TttBoard, TttPlayer, std::vector<TttAction>>>
      augmented = internal::AugmentAll(board, player, actions);

  std::vector<
      std::tuple<TttBoard, TttPlayer, std::vector<TttAction>, PolicyOutput>>
      result;
  result.reserve(augmented.size());

  for (auto&& [key, game] : augmented) {
    auto& [aug_board, aug_player, aug_actions] = game;
    result.emplace_back(std::move(aug_board), player, std::move(aug_actions),
                        output);
  }

  return result;
}

}  // namespace alphazero::game::api::test
