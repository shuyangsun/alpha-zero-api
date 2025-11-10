#ifndef ALPHA_ZERO_API_TEST_CONNECT4_SERIALIZER_H_
#define ALPHA_ZERO_API_TEST_CONNECT4_SERIALIZER_H_

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "alpha-zero-api/serializer.h"
#include "game.h"

namespace alphazero::game::api::test {

class C4Serializer : public IGameSerializer<C4Board, C4Action, C4Player> {
 public:
  C4Serializer() = default;
  ~C4Serializer() override = default;

  std::vector<float> Serialize(const C4Board& board, const C4Player& player,
                               std::span<const C4Action> actions) const final;
};

}  // namespace alphazero::game::api::test

#endif  // ALPHA_ZERO_API_TEST_CONNECT4_SERIALIZER_H_
