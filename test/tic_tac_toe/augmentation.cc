#include "augmentation.h"

#include <cstdint>
#include <cstdlib>
#include <span>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "game.h"

namespace az::game::api::test::internal {

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

std::unordered_map<uint8_t,
                   std::tuple<TttBoard, TttPlayer, std::vector<TttAction>>>
AugmentAll(const TttBoard& board, const TttPlayer& player,
           std::span<const TttAction> actions) {
  using enum Augmentation;
  std::unordered_map<uint8_t,
                     std::tuple<TttBoard, TttPlayer, std::vector<TttAction>>>
      result;
  result.reserve(12);
  result.emplace(
      static_cast<uint8_t>(kOriginal),
      std::make_tuple(board, player,
                      std::vector<TttAction>(actions.begin(), actions.end())));

  // Rotations.
  const auto [rotated_board_1, rotated_player_1, rotated_actions_1] =
      RotateClockwise(board, player, actions);
  result.emplace(
      static_cast<uint8_t>(kRotate90),
      std::make_tuple(rotated_board_1, rotated_player_1, rotated_actions_1));

  const auto [rotated_board_2, rotated_player_2, rotated_actions_2] =
      RotateClockwise(rotated_board_1, rotated_player_1, rotated_actions_1);
  result.emplace(
      static_cast<uint8_t>(kRotate180),
      std::make_tuple(rotated_board_2, rotated_player_2, rotated_actions_2));

  const auto [rotated_board_3, rotated_player_3, rotated_actions_3] =
      RotateClockwise(rotated_board_2, rotated_player_2, rotated_actions_2);
  result.emplace(
      static_cast<uint8_t>(kRotate270),
      std::make_tuple(rotated_board_3, rotated_player_3, rotated_actions_3));

  // Mirror horizontally.
  const auto [mirrored_board_h, mirrored_player_h, mirrored_actions_h] =
      MirrorHorizontal(board, player, actions);
  result.emplace(
      static_cast<uint8_t>(kMirrorHorizontal),
      std::make_tuple(mirrored_board_h, mirrored_player_h, mirrored_actions_h));

  // Mirror horizontally + rotations.
  const auto [mh_rotated_board_1, mh_rotated_player_1, mh_rotated_actions_1] =
      RotateClockwise(mirrored_board_h, mirrored_player_h, mirrored_actions_h);
  result.emplace(static_cast<uint8_t>(kMirrorHorizontalRotate90),
                 std::make_tuple(mh_rotated_board_1, mh_rotated_player_1,
                                 mh_rotated_actions_1));

  const auto [mh_rotated_board_2, mh_rotated_player_2, mh_rotated_actions_2] =
      RotateClockwise(mh_rotated_board_1, mh_rotated_player_1,
                      mh_rotated_actions_1);
  result.emplace(static_cast<uint8_t>(kMirrorHorizontalRotate180),
                 std::make_tuple(mh_rotated_board_2, mh_rotated_player_2,
                                 mh_rotated_actions_2));

  const auto [mh_rotated_board_3, mh_rotated_player_3, mh_rotated_actions_3] =
      RotateClockwise(mh_rotated_board_2, mh_rotated_player_2,
                      mh_rotated_actions_2);
  result.emplace(static_cast<uint8_t>(kMirrorHorizontalRotate270),
                 std::make_tuple(mh_rotated_board_3, mh_rotated_player_3,
                                 mh_rotated_actions_3));

  // Mirror vertically.
  const auto [mirrored_board_v, mirrored_player_v, mirrored_actions_v] =
      MirrorVertical(board, player, actions);
  result.emplace(
      static_cast<uint8_t>(kMirrorVertical),
      std::make_tuple(mirrored_board_v, mirrored_player_v, mirrored_actions_v));

  // Mirror vertically + rotations.
  const auto [mv_rotated_board_1, mv_rotated_player_1, mv_rotated_actions_1] =
      RotateClockwise(mirrored_board_v, mirrored_player_v, mirrored_actions_v);
  result.emplace(static_cast<uint8_t>(kMirrorVerticalRotate90),
                 std::make_tuple(mv_rotated_board_1, mv_rotated_player_1,
                                 mv_rotated_actions_1));

  const auto [mv_rotated_board_2, mv_rotated_player_2, mv_rotated_actions_2] =
      RotateClockwise(mv_rotated_board_1, mv_rotated_player_1,
                      mv_rotated_actions_1);
  result.emplace(static_cast<uint8_t>(kMirrorVerticalRotate180),
                 std::make_tuple(mv_rotated_board_2, mv_rotated_player_2,
                                 mv_rotated_actions_2));

  const auto [mv_rotated_board_3, mv_rotated_player_3, mv_rotated_actions_3] =
      RotateClockwise(mv_rotated_board_2, mv_rotated_player_2,
                      mv_rotated_actions_2);
  result.emplace(static_cast<uint8_t>(kMirrorVerticalRotate270),
                 std::make_tuple(mv_rotated_board_3, mv_rotated_player_3,
                                 mv_rotated_actions_3));
  return result;
}

}  // namespace az::game::api::test::internal
