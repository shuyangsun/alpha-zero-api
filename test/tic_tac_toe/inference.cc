#include "inference.h"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <span>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "alpha-zero-api/policy_output.h"
#include "augmentation.h"
#include "game.h"

namespace alphazero::game::api::test {

namespace {

using ::alphazero::game::api::test::internal::Augmentation;
using ::alphazero::game::api::test::internal::MirrorHorizontal;
using ::alphazero::game::api::test::internal::MirrorVertical;
using ::alphazero::game::api::test::internal::RotateClockwise;
using ::alphazero::game::api::test::internal::RotateCounterclockwise;

}  // namespace

std::unordered_map<uint8_t,
                   std::tuple<TttBoard, TttPlayer, std::vector<TttAction>>>
TttInferenceAugmenter::Augment(const TttBoard& board, const TttPlayer& player,
                               std::span<const TttAction> actions) const {
  using enum Augmentation;
  std::unordered_map<uint8_t,
                     std::tuple<TttBoard, TttPlayer, std::vector<TttAction>>>
      result;
  result.reserve(12);
  result.emplace(
      static_cast<uint8_t>(kOriginal),
      std::make_tuple(board, player,
                      std::vector<TttAction>(actions.begin(), actions.end())));

  // Rotatations.
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

PolicyOutput TttInferenceAugmenter::Interpret(
    const std::unordered_map<
        uint8_t, std::tuple<TttBoard, TttPlayer, std::vector<TttAction>>>&
        augmented_games,
    const std::unordered_map<uint8_t, PolicyOutput>& outputs) const {
  using enum Augmentation;

  assert(augmented_games.find(static_cast<uint8_t>(kOriginal)) !=
         augmented_games.end());
  assert(outputs.find(static_cast<uint8_t>(kOriginal)) != outputs.end());

  std::unordered_map<uint8_t, size_t> action_index_map;
  std::span<const TttAction> original_actions = std::get<2>(
      augmented_games.find(static_cast<uint8_t>(kOriginal))->second);
  action_index_map.reserve(original_actions.size());
  for (size_t i = 0; i < original_actions.size(); ++i) {
    const TttAction& action = original_actions[i];
    action_index_map.emplace(action.row * TTT_COLS + action.col, i);
  }

  assert(augmented_games.size() == outputs.size());

  float values_sum = 0.0f;

  std::vector<float> probs;

  for (const auto& [key, game] : augmented_games) {
    const auto& [board, player, actions] = game;
    values_sum += outputs.at(key).value;

    const PolicyOutput& policy_output = outputs.at(key);

    for (size_t i = 0; i < actions.size(); ++i) {
      if (probs.empty()) {
        probs.resize(actions.size(), 0.0f);
      } else {
        assert(probs.size() == actions.size());
      }
      TttAction original_action = actions.at(i);
      switch (static_cast<Augmentation>(key)) {
        case kOriginal:
          break;
        case kRotate90:
          original_action = RotateCounterclockwise(original_action, 1);
          break;
        case kRotate180:
          original_action = RotateCounterclockwise(original_action, 2);
          break;
        case kRotate270:
          original_action = RotateCounterclockwise(original_action, 3);
          break;
        case kMirrorHorizontal:
          original_action = MirrorHorizontal(original_action);
          break;
        case kMirrorHorizontalRotate90:
          original_action =
              MirrorHorizontal(RotateCounterclockwise(original_action, 1));
          break;
        case kMirrorHorizontalRotate180:
          original_action =
              MirrorHorizontal(RotateCounterclockwise(original_action, 2));
          break;
        case kMirrorHorizontalRotate270:
          original_action =
              MirrorHorizontal(RotateCounterclockwise(original_action, 3));
          break;
        case kMirrorVertical:
          original_action = MirrorVertical(original_action);
          break;
        case kMirrorVerticalRotate90:
          original_action =
              MirrorVertical(RotateCounterclockwise(original_action, 1));
          break;
        case kMirrorVerticalRotate180:
          original_action =
              MirrorVertical(RotateCounterclockwise(original_action, 2));
          break;
        case kMirrorVerticalRotate270:
          original_action =
              MirrorVertical(RotateCounterclockwise(original_action, 3));
          break;
      }
      assert(action_index_map.find(original_action.row * TTT_COLS +
                                   original_action.col) !=
             action_index_map.end());
      probs[action_index_map.at(original_action.row * TTT_COLS +
                                original_action.col)] +=
          policy_output.probabilities[i] / augmented_games.size();
    }
  }

  return PolicyOutput(values_sum / augmented_games.size(), std::move(probs));
}

}  // namespace alphazero::game::api::test
