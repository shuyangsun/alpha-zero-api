#include "connect4/serializer.h"

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "game.h"

namespace alphazero::game::api::test {

std::vector<float> C4Serializer::Serialize(
    const C4Board& board, const C4Player& player,
    std::span<const C4Action> actions) const {
  std::vector<float> result;
  result.reserve(C4_ROWS * C4_COLS);
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
