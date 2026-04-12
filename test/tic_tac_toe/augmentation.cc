#include "augmentation.h"

#include <cstdlib>
#include <span>
#include <tuple>
#include <vector>

#include "game.h"

namespace alphazero::game::api::test::internal {

TttAction MirrorHorizontal(const TttAction& action) {
  return TttAction{action.row,
                   static_cast<uint16_t>(TTT_COLS - 1 - action.col)};
}

TttAction MirrorVertical(const TttAction& action) {
  return TttAction{static_cast<uint16_t>(TTT_ROWS - 1 - action.row),
                   action.col};
}

TttAction RotateClockwise(const TttAction& action) {
  return TttAction{static_cast<uint16_t>(action.col),
                   static_cast<uint16_t>(TTT_ROWS - 1 - action.row)};
}

TttAction RotateCounterclockwise(const TttAction& action, size_t times) {
  times = times % 4;
  switch (times) {
    case 1:
      return TttAction{static_cast<uint16_t>(TTT_COLS - 1 - action.col),
                       static_cast<uint16_t>(action.row)};
    case 2:
      return TttAction{static_cast<uint16_t>(TTT_ROWS - 1 - action.row),
                       static_cast<uint16_t>(TTT_COLS - 1 - action.col)};
    case 3:
      return RotateClockwise(action);
    default:
      return action;
  }
}

std::tuple<TttBoard, TttPlayer, std::vector<TttAction>> MirrorHorizontal(
    const TttBoard& board, const TttPlayer& player,
    std::span<const TttAction> actions) {
  TttBoard mirrored_board = board;
  for (size_t row = 0; row < TTT_ROWS; ++row) {
    for (size_t col = 0; col < TTT_COLS / 2; ++col) {
      std::swap(mirrored_board[row][col],
                mirrored_board[row][TTT_COLS - 1 - col]);
    }
  }
  std::vector<TttAction> mirrored_actions;
  mirrored_actions.reserve(actions.size());
  for (const auto& action : actions) {
    mirrored_actions.emplace_back(MirrorHorizontal(action));
  }
  return std::make_tuple(mirrored_board, player, mirrored_actions);
}

std::tuple<TttBoard, TttPlayer, std::vector<TttAction>> MirrorVertical(
    const TttBoard& board, const TttPlayer& player,
    std::span<const TttAction> actions) {
  TttBoard mirrored_board = board;
  for (size_t row = 0; row < TTT_ROWS / 2; ++row) {
    for (size_t col = 0; col < TTT_COLS; ++col) {
      std::swap(mirrored_board[row][col],
                mirrored_board[TTT_ROWS - 1 - row][col]);
    }
  }
  std::vector<TttAction> mirrored_actions;
  mirrored_actions.reserve(actions.size());
  for (const auto& action : actions) {
    mirrored_actions.emplace_back(MirrorVertical(action));
  }
  return std::make_tuple(mirrored_board, player, mirrored_actions);
}

std::tuple<TttBoard, TttPlayer, std::vector<TttAction>> RotateClockwise(
    const TttBoard& board, const TttPlayer& player,
    std::span<const TttAction> actions) {
  TttBoard rotated_board = board;
  TttBoard temp = rotated_board;
  for (size_t row = 0; row < TTT_ROWS; ++row) {
    for (size_t col = 0; col < TTT_COLS; ++col) {
      rotated_board[col][TTT_ROWS - 1 - row] = temp[row][col];
    }
  }
  std::vector<TttAction> rotated_actions;
  rotated_actions.reserve(actions.size());
  for (const auto& action : actions) {
    rotated_actions.emplace_back(RotateClockwise(action));
  }
  return std::make_tuple(rotated_board, player, rotated_actions);
}

}  // namespace alphazero::game::api::test::internal
