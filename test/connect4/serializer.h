#ifndef ALPHA_ZERO_API_TEST_CONNECT4_SERIALIZER_H_
#define ALPHA_ZERO_API_TEST_CONNECT4_SERIALIZER_H_

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "alpha-zero-api/serializer.h"

namespace alphazero::game::api::test {

class Connect4Serializer
    : public ISerializer<std::array<int8_t, 4>, int8_t, bool> {
 public:
  Connect4Serializer() = default;
  ~Connect4Serializer() override = default;

  std::vector<float> Serialize(const std::array<int8_t, 4>& board,
                               const bool& player,
                               std::span<const int8_t> actions) const final;
};

}  // namespace alphazero::game::api::test

#endif  // ALPHA_ZERO_API_TEST_CONNECT4_SERIALIZER_H_
