#ifndef ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_GAME_H_
#define ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_GAME_H_

#include <array>
#include <cstdint>
#include <vector>

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

}  // namespace az::game::api

#endif  // ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_GAME_H_
