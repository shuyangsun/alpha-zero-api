#include "connect4/serializer.h"

#include <array>
#include <cstdint>
#include <span>
#include <vector>

namespace alphazero::game::api::test {

std::vector<float> Connect4Serializer::Serialize(
    const std::array<int8_t, 4>& board, const bool& player,
    std::span<const int8_t> actions) const {
  return std::vector<float>{};  // TODO
}

}  // namespace alphazero::game::api::test
