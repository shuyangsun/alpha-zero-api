#include "tic_tac_toe/serializer.h"

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "game.h"

namespace az::game::api::test {

std::vector<float> TttSerializer::SerializeCurrentState(
    const TttBoard& board, const TttPlayer& player,
    std::span<const TttAction> actions) const {
  std::vector<float> result;
  result.reserve(TTT_ROWS * TTT_COLS);
  for (const auto& row : board) {
    for (const auto& cell : row) {
      // Flip color if player 1.
      result.emplace_back(static_cast<float>(player ? -cell : cell));
    }
  }
  return result;
}

std::vector<float> TttSerializer::SerializePolicyOutput(
    const TttBoard& board, const TttPlayer& player,
    std::span<const TttAction> actions, const PolicyOutput& output) const {
  std::vector<float> result(TTT_COLS * TTT_ROWS + 1, 0.0f);
  result[0] = output.value;
  for (size_t i = 0; i < actions.size(); ++i) {
    const TttAction& action = actions[i];
    const size_t index = action.row * TTT_COLS + action.col;
    result[index + 1] = output.probabilities[i];
  }
  return result;
}

}  // namespace az::game::api::test
