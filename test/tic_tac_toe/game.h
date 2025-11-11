#ifndef ALPHA_ZERO_API_TEST_TIC_TAC_TOE_GAME_H_
#define ALPHA_ZERO_API_TEST_TIC_TAC_TOE_GAME_H_

#include "alpha-zero-api/defaults/game.h"

namespace alphazero::game::api::test {

constexpr uint16_t TTT_ROWS = 3;
constexpr uint16_t TTT_COLS = 3;
using TttBoard = Standard2DBoard<TTT_ROWS, TTT_COLS>;
using TttAction = Action2D;
using TttPlayer = BinaryPlayer;

}  // namespace alphazero::game::api::test

#endif  // ALPHA_ZERO_API_TEST_TIC_TAC_TOE_GAME_H_
