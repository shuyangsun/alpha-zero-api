#include "inference.h"

#include <span>
#include <tuple>
#include <vector>

#include "game.h"

namespace alphazero::game::api::test {

std::vector<std::tuple<TttBoard, TttPlayer, std::vector<TttAction>>>
TttInferenceAugmenter::Augment(const TttBoard& board, const TttPlayer& player,
                               std::span<const TttAction> actions) const {
  return {};  // TODO: implementation
}

std::vector<float> TttInferenceAugmenter::Interpret(
    std::span<const std::tuple<TttBoard, TttPlayer, std::vector<TttAction>>>
        augmented_games,
    std::span<const std::vector<float>> outputs) const {
  return {0.0f, {}};  // TODO: implementation
}

}  // namespace alphazero::game::api::test
