#ifndef ALPHA_ZERO_API_TEST_CONNECT4_GAME_H_
#define ALPHA_ZERO_API_TEST_CONNECT4_GAME_H_

#include "alpha-zero-api/defaults/game.h"

namespace alphazero::game::api::test {

constexpr uint16_t C4_ROWS = 4;
constexpr uint16_t C4_COLS = 4;
using C4Board = Standard2DBoard<C4_ROWS, C4_COLS>;
using C4Action = Action2D;
using C4Player = BinaryPlayer;

}  // namespace alphazero::game::api::test

#endif  // ALPHA_ZERO_API_TEST_CONNECT4_GAME_H_
