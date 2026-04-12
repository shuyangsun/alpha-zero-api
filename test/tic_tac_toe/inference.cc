#include "inference.h"

#include <span>
#include <tuple>
#include <vector>

#include "game.h"

namespace alphazero::game::api::test {

namespace {

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
    mirrored_actions.emplace_back(TttAction{
        action.row, static_cast<uint16_t>(TTT_COLS - 1 - action.col)});
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
    mirrored_actions.emplace_back(TttAction{
        static_cast<uint16_t>(TTT_ROWS - 1 - action.row), action.col});
  }
  return std::make_tuple(mirrored_board, player, mirrored_actions);
}

std::tuple<TttBoard, TttPlayer, std::vector<TttAction>> RotateOnceClockwise(
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
    rotated_actions.emplace_back(
        TttAction{static_cast<uint16_t>(action.col),
                  static_cast<uint16_t>(TTT_ROWS - 1 - action.row)});
  }
  return std::make_tuple(rotated_board, player, rotated_actions);
}

}  // namespace

std::vector<std::tuple<TttBoard, TttPlayer, std::vector<TttAction>>>
TttInferenceAugmenter::Augment(const TttBoard& board, const TttPlayer& player,
                               std::span<const TttAction> actions) const {
  std::vector<std::tuple<TttBoard, TttPlayer, std::vector<TttAction>>> result;
  result.reserve(12);
  result.emplace_back(board, player,
                      std::vector<TttAction>(actions.begin(), actions.end()));

  // Rotatations.
  const auto [rotated_board_1, rotated_player_1, rotated_actions_1] =
      RotateOnceClockwise(board, player, actions);
  result.emplace_back(rotated_board_1, rotated_player_1, rotated_actions_1);

  const auto [rotated_board_2, rotated_player_2, rotated_actions_2] =
      RotateOnceClockwise(rotated_board_1, rotated_player_1, rotated_actions_1);
  result.emplace_back(rotated_board_2, rotated_player_2, rotated_actions_2);

  const auto [rotated_board_3, rotated_player_3, rotated_actions_3] =
      RotateOnceClockwise(rotated_board_2, rotated_player_2, rotated_actions_2);
  result.emplace_back(rotated_board_3, rotated_player_3, rotated_actions_3);

  // Mirror horizontally.
  const auto [mirrored_board_h, mirrored_player_h, mirrored_actions_h] =
      MirrorHorizontal(board, player, actions);
  result.emplace_back(mirrored_board_h, mirrored_player_h, mirrored_actions_h);

  // Mirror horizontally + rotations.
  const auto [mh_rotated_board_1, mh_rotated_player_1, mh_rotated_actions_1] =
      RotateOnceClockwise(mirrored_board_h, mirrored_player_h,
                          mirrored_actions_h);
  result.emplace_back(mh_rotated_board_1, mh_rotated_player_1,
                      mh_rotated_actions_1);

  const auto [mh_rotated_board_2, mh_rotated_player_2, mh_rotated_actions_2] =
      RotateOnceClockwise(mh_rotated_board_1, mh_rotated_player_1,
                          mh_rotated_actions_1);
  result.emplace_back(mh_rotated_board_2, mh_rotated_player_2,
                      mh_rotated_actions_2);

  const auto [mh_rotated_board_3, mh_rotated_player_3, mh_rotated_actions_3] =
      RotateOnceClockwise(mh_rotated_board_2, mh_rotated_player_2,
                          mh_rotated_actions_2);
  result.emplace_back(mh_rotated_board_3, mh_rotated_player_3,
                      mh_rotated_actions_3);

  // Mirror vertically.
  const auto [mirrored_board_v, mirrored_player_v, mirrored_actions_v] =
      MirrorVertical(board, player, actions);
  result.emplace_back(mirrored_board_v, mirrored_player_v, mirrored_actions_v);

  // Mirror vertically + rotations.
  const auto [mv_rotated_board_1, mv_rotated_player_1, mv_rotated_actions_1] =
      RotateOnceClockwise(mirrored_board_v, mirrored_player_v,
                          mirrored_actions_v);
  result.emplace_back(mv_rotated_board_1, mv_rotated_player_1,
                      mv_rotated_actions_1);

  const auto [mv_rotated_board_2, mv_rotated_player_2, mv_rotated_actions_2] =
      RotateOnceClockwise(mv_rotated_board_1, mv_rotated_player_1,
                          mv_rotated_actions_1);
  result.emplace_back(mv_rotated_board_2, mv_rotated_player_2,
                      mv_rotated_actions_2);

  const auto [mv_rotated_board_3, mv_rotated_player_3, mv_rotated_actions_3] =
      RotateOnceClockwise(mv_rotated_board_2, mv_rotated_player_2,
                          mv_rotated_actions_2);
  result.emplace_back(mv_rotated_board_3, mv_rotated_player_3,
                      mv_rotated_actions_3);
  return result;
}

std::vector<float> TttInferenceAugmenter::Interpret(
    std::span<const std::tuple<TttBoard, TttPlayer, std::vector<TttAction>>>
        augmented_games,
    std::span<const std::vector<float>> outputs) const {
  return {0.0f, {}};  // TODO: implementation
}

}  // namespace alphazero::game::api::test
