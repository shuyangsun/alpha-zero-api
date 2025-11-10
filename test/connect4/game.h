#ifndef ALPHA_ZERO_API_TEST_CONNECT4_GAME_H_
#define ALPHA_ZERO_API_TEST_CONNECT4_GAME_H_

#include "alpha-zero-api/defaults/game.h"

namespace alphazero::game::api::test {

using C4Board = Standard2DBoard<4, 4>;
using C4Action = Action2D;
using C4Player = BinaryPlayer;

}  // namespace alphazero::game::api::test

#endif  // ALPHA_ZERO_API_TEST_CONNECT4_GAME_H_
