#include "tic_tac_toe/serializer.h"

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "game.h"

namespace alphazero::game::api::test {

std::vector<float> TttSerializer::SerializeCurrentState(
    const TttBoard& board, const TttPlayer& player,
    std::span<const TttAction> actions) const {
  std::vector<float> result;
  result.reserve(TTT_ROWS * TTT_COLS + 1);
  result.emplace_back(player ? 1.0f : 0.0f);
  for (const auto& row : board) {
    for (const auto& cell : row) {
      // Flip color if player 1.
      result.push_back(static_cast<float>(player ? -cell : cell));
    }
  }
  return result;
}

}  // namespace alphazero::game::api::test
