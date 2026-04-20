#ifndef ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_GAME_H_
#define ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_GAME_H_

#include <array>
#include <vector>

#include "alpha-zero-api/game.h"

namespace az::game::api {

// Standard game boards
template <uint16_t SZ>
using Standard1DBoard = std::array<int8_t, SZ>;
using Standard1DLargeBoard = std::vector<int8_t>;
template <uint16_t ROWS, uint16_t COLS>
using Standard2DBoard = std::array<std::array<int8_t, COLS>, ROWS>;
using Standard2DLargeBoard = std::vector<std::vector<int8_t>>;

// Standard action types
using Action1D = int16_t;
struct Action2D {
  uint16_t row;
  uint16_t col;
};

// Standard player types
using BinaryPlayer = bool;
constexpr BinaryPlayer Player1 = false;
constexpr BinaryPlayer Player2 = true;

// Standard game interfaces
template <uint16_t SZ, typename E>
using ITwoPlayer1DBoardGame =
    IGame<Standard1DBoard<SZ>, Action1D, BinaryPlayer, E>;
template <typename E>
using ITwoPlayer1DLargeBoardGame =
    IGame<Standard1DLargeBoard, Action1D, BinaryPlayer, E>;

template <uint16_t ROWS, uint16_t COLS, typename E>
using ITwoPlayer2DBoardGame =
    IGame<Standard2DBoard<ROWS, COLS>, Action2D, BinaryPlayer, E>;
template <typename E>
using ITwoPlayer2DLargeBoardGame =
    IGame<Standard2DLargeBoard, Action2D, BinaryPlayer, E>;

}  // namespace az::game::api

#endif  // ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_GAME_H_
