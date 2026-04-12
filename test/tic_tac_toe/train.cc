#include "train.h"

#include <span>
#include <tuple>
#include <vector>

#include "game.h"

namespace alphazero::game::api::test {

std::vector<
    std::tuple<TttBoard, TttPlayer, std::vector<TttAction>, PolicyOutput>>
TttTrainingAugmenter::Augment(const TttBoard& board, const TttPlayer& player,
                              std::span<const TttAction> actions,
                              PolicyOutput&& output) const {
  return {};  // TODO: implementation
}

}  // namespace alphazero::game::api::test
