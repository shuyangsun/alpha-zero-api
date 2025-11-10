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
  return std::vector<float>{};  // TODO
}

}  // namespace alphazero::game::api::test
