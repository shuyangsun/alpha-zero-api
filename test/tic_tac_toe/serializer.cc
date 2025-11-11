#include "tic_tac_toe/serializer.h"

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "game.h"

namespace alphazero::game::api::test {

std::vector<float> TttSerializer::Serialize(
    const TttBoard& board, const TttPlayer& player,
    std::span<const TttAction> actions) const {
  std::vector<float> result;
  result.reserve(TTT_ROWS * TTT_COLS);
  for (const auto& row : board) {
    for (const auto& cell : row) {
      if (player) {
        // Flip color if player 1.
        result.push_back(static_cast<float>(-cell));
      } else {
        // Keep color if player 0.
        result.push_back(static_cast<float>(cell));
      }
    }
  }
  return result;
}

}  // namespace alphazero::game::api::test
