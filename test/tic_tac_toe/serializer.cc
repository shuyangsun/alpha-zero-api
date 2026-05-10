#include "tic_tac_toe/serializer.h"

#include <vector>

#include "alpha-zero-api/ring_buffer.h"
#include "tic_tac_toe/game.h"

namespace az::game::api::test {

std::vector<float> TttSerializer::SerializeCurrentState(
    const TttGame& game, RingBufferView<TttGame> /*history*/) const noexcept {
  const TttBoard& board = game.GetBoard();
  const TttPlayer player = game.CurrentPlayer();
  std::vector<float> result;
  result.reserve(TTT_CELLS);
  for (const auto& row : board) {
    for (const auto& cell : row) {
      result.emplace_back(static_cast<float>(player ? -cell : cell));
    }
  }
  return result;
}

}  // namespace az::game::api::test
