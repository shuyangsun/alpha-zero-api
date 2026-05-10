#include "augmentation.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include "tic_tac_toe/game.h"

namespace az::game::api::test::internal {

namespace {

[[nodiscard]] TttBoard MirrorHorizontalBoard(const TttBoard& board) noexcept {
  TttBoard result = board;
  for (std::size_t row = 0; row < TTT_ROWS; ++row) {
    for (std::size_t col = 0; col < TTT_COLS / 2; ++col) {
      std::swap(result[row][col], result[row][TTT_COLS - 1 - col]);
    }
  }
  return result;
}

[[nodiscard]] TttBoard MirrorVerticalBoard(const TttBoard& board) noexcept {
  TttBoard result = board;
  for (std::size_t row = 0; row < TTT_ROWS / 2; ++row) {
    for (std::size_t col = 0; col < TTT_COLS; ++col) {
      std::swap(result[row][col], result[TTT_ROWS - 1 - row][col]);
    }
  }
  return result;
}

[[nodiscard]] TttBoard RotateClockwiseBoard(const TttBoard& board) noexcept {
  TttBoard result = board;
  for (std::size_t row = 0; row < TTT_ROWS; ++row) {
    for (std::size_t col = 0; col < TTT_COLS; ++col) {
      result[col][TTT_ROWS - 1 - row] = board[row][col];
    }
  }
  return result;
}

[[nodiscard]] std::optional<TttAction> TransformLastAction(
    const std::optional<TttAction>& last,
    TttAction (*transform)(const TttAction&)) noexcept {
  if (!last.has_value()) {
    return std::nullopt;
  }
  return transform(*last);
}

}  // namespace

TttAction MirrorHorizontal(const TttAction& action) noexcept {
  return TttAction{action.row,
                   static_cast<uint16_t>(TTT_COLS - 1 - action.col)};
}

TttAction MirrorVertical(const TttAction& action) noexcept {
  return TttAction{static_cast<uint16_t>(TTT_ROWS - 1 - action.row),
                   action.col};
}

TttAction RotateClockwise(const TttAction& action) noexcept {
  return TttAction{static_cast<uint16_t>(action.col),
                   static_cast<uint16_t>(TTT_ROWS - 1 - action.row)};
}

TttAction RotateCounterclockwise(const TttAction& action,
                                 std::size_t times) noexcept {
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

TttGame MirrorHorizontal(const TttGame& game) noexcept {
  return TttGame{
      MirrorHorizontalBoard(game.GetBoard()), game.CurrentPlayer(),
      game.CurrentRound(),
      TransformLastAction(
          game.LastAction(),
          static_cast<TttAction (*)(const TttAction&)>(&MirrorHorizontal))};
}

TttGame MirrorVertical(const TttGame& game) noexcept {
  return TttGame{
      MirrorVerticalBoard(game.GetBoard()), game.CurrentPlayer(),
      game.CurrentRound(),
      TransformLastAction(
          game.LastAction(),
          static_cast<TttAction (*)(const TttAction&)>(&MirrorVertical))};
}

TttGame RotateClockwise(const TttGame& game) noexcept {
  return TttGame{
      RotateClockwiseBoard(game.GetBoard()), game.CurrentPlayer(),
      game.CurrentRound(),
      TransformLastAction(
          game.LastAction(),
          static_cast<TttAction (*)(const TttAction&)>(&RotateClockwise))};
}

std::vector<TttGame> AugmentAll(const TttGame& game) noexcept {
  std::vector<TttGame> result;
  result.reserve(kNumAugmentations);

  result.emplace_back(game);
  TttGame r1 = RotateClockwise(game);
  TttGame r2 = RotateClockwise(r1);
  TttGame r3 = RotateClockwise(r2);
  result.emplace_back(r1);
  result.emplace_back(r2);
  result.emplace_back(r3);

  TttGame mh = MirrorHorizontal(game);
  TttGame mh1 = RotateClockwise(mh);
  TttGame mh2 = RotateClockwise(mh1);
  TttGame mh3 = RotateClockwise(mh2);
  result.emplace_back(std::move(mh));
  result.emplace_back(std::move(mh1));
  result.emplace_back(std::move(mh2));
  result.emplace_back(std::move(mh3));

  TttGame mv = MirrorVertical(game);
  TttGame mv1 = RotateClockwise(mv);
  TttGame mv2 = RotateClockwise(mv1);
  TttGame mv3 = RotateClockwise(mv2);
  result.emplace_back(std::move(mv));
  result.emplace_back(std::move(mv1));
  result.emplace_back(std::move(mv2));
  result.emplace_back(std::move(mv3));

  return result;
}

TttAction InverseTransformAction(const TttAction& augmented_action,
                                 Augmentation augmentation) noexcept {
  using enum Augmentation;
  switch (augmentation) {
    case kOriginal:
      return augmented_action;
    case kRotate90:
      return RotateCounterclockwise(augmented_action, 1);
    case kRotate180:
      return RotateCounterclockwise(augmented_action, 2);
    case kRotate270:
      return RotateCounterclockwise(augmented_action, 3);
    case kMirrorHorizontal:
      return MirrorHorizontal(augmented_action);
    case kMirrorHorizontalRotate90:
      return MirrorHorizontal(RotateCounterclockwise(augmented_action, 1));
    case kMirrorHorizontalRotate180:
      return MirrorHorizontal(RotateCounterclockwise(augmented_action, 2));
    case kMirrorHorizontalRotate270:
      return MirrorHorizontal(RotateCounterclockwise(augmented_action, 3));
    case kMirrorVertical:
      return MirrorVertical(augmented_action);
    case kMirrorVerticalRotate90:
      return MirrorVertical(RotateCounterclockwise(augmented_action, 1));
    case kMirrorVerticalRotate180:
      return MirrorVertical(RotateCounterclockwise(augmented_action, 2));
    case kMirrorVerticalRotate270:
      return MirrorVertical(RotateCounterclockwise(augmented_action, 3));
  }
  return augmented_action;
}

}  // namespace az::game::api::test::internal
