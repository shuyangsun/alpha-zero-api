#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <span>
#include <utility>
#include <vector>

#include "alpha-zero-api/defaults/compact_deserializer.h"
#include "alpha-zero-api/defaults/compact_serializer.h"
#include "alpha-zero-api/defaults/serializer.h"
#include "alpha-zero-api/policy_output.h"
#include "alpha-zero-api/ring_buffer.h"
#include "tic_tac_toe/deserializer.h"
#include "tic_tac_toe/game.h"
#include "tic_tac_toe/inference.h"
#include "tic_tac_toe/serializer.h"
#include "tic_tac_toe/train.h"

namespace {

using ::az::game::api::CompactPolicyOutputBlob;
using ::az::game::api::CompactPolicyTargetBlob;
using ::az::game::api::DefaultCompactPolicyOutputDeserializer;
using ::az::game::api::DefaultCompactPolicyOutputSerializer;
using ::az::game::api::DefaultPolicyOutputSerializer;
using ::az::game::api::Evaluation;
using ::az::game::api::RingBuffer;
using ::az::game::api::TrainingTarget;

using ::az::game::api::test::TttAction;
using ::az::game::api::test::TttDeserializer;
using ::az::game::api::test::TttGame;
using ::az::game::api::test::TttInferenceAugmenter;
using ::az::game::api::test::TttResult;
using ::az::game::api::test::TttSerializer;
using ::az::game::api::test::TttTrainingAugmenter;

// History buffer for a Markov game (kHistoryLookback == 0). Storage is
// `std::array<TttGame, 0>` so the buffer occupies essentially no
// space; `View()` always returns an empty view.
using TttHistory =
    RingBuffer<TttGame, std::array<TttGame, TttGame::kHistoryLookback>>;

}  // namespace

int main() {
  TttGame game{};
  game.ApplyActionInPlace(TttAction{1, 1});
  game.ApplyActionInPlace(TttAction{0, 1});
  game.ApplyActionInPlace(TttAction{2, 0});

  std::cout << "Current board:\n" << game.BoardReadableString() << "\n";

  // Step 1: augment the current state for inference.
  const TttInferenceAugmenter inference;
  const std::vector<TttGame> augmented_games = inference.Augment(game);
  if (augmented_games.empty()) {
    std::cerr << "Error: augmenter returned no games." << std::endl;
    return 1;
  }
  std::cout << "Number of augmented games: " << augmented_games.size()
            << std::endl;

  // Step 2: serialize each augmented state to the network input layout.
  const TttSerializer serializer;
  const TttHistory history{};
  std::vector<std::vector<float>> augmented_game_states;
  augmented_game_states.reserve(augmented_games.size());
  for (const TttGame& aug_game : augmented_games) {
    augmented_game_states.emplace_back(
        serializer.SerializeCurrentState(aug_game, history.View()));
    if (augmented_game_states.size() > 1 &&
        augmented_game_states.back().size() !=
            augmented_game_states.front().size()) {
      std::cerr << "Error: Augmented game state vector size is not the "
                   "same across augmented games."
                << std::endl;
      return 1;
    }
  }
  std::cout << "Serialized state vector length: "
            << augmented_game_states.front().size() << std::endl;

  // Step 3: simulate one round of MCTS — for each augmented game, fake
  // a uniform-prior `TrainingTarget`, run it through the round-trip
  // policy serializer + deserializer, and collect `Evaluation`s.
  const DefaultPolicyOutputSerializer<TttGame> policy_serializer;
  const TttDeserializer deserializer;

  std::vector<Evaluation> augmented_evaluations;
  augmented_evaluations.reserve(augmented_games.size());
  for (const TttGame& aug_game : augmented_games) {
    std::array<TttAction, TttGame::kMaxLegalActions> aug_actions{};
    const std::size_t aug_count = aug_game.ValidActionsInto(aug_actions);
    const TrainingTarget fake_target{
        0.5f,
        std::vector<float>(aug_count, 1.0f / static_cast<float>(aug_count))};
    const std::vector<float> nn_output =
        policy_serializer.SerializePolicyOutput(aug_game, fake_target);
    const TttResult<Evaluation> decoded =
        deserializer.Deserialize(aug_game, nn_output);
    if (!decoded.has_value()) {
      std::cerr << "Error: deserializer rejected its own serializer's output."
                << std::endl;
      return 1;
    }
    assert(decoded->probabilities.size() == aug_count);
    augmented_evaluations.push_back(*decoded);
  }
  std::cout << "Serialized policy output vector length: "
            << TttGame::kPolicySize + 1 << std::endl;

  // Step 4: combine the per-augmentation evaluations back into one for
  // the original game.
  const Evaluation combined =
      inference.Interpret(game, std::span<const TttGame>{augmented_games},
                          std::span<const Evaluation>{augmented_evaluations});

  std::array<TttAction, TttGame::kMaxLegalActions> valid_actions{};
  const std::size_t valid_count = game.ValidActionsInto(valid_actions);
  TttAction next = TttAction{0, 0};
  float max_prob = -1.0f;
  for (std::size_t i = 0; i < valid_count; ++i) {
    if (combined.probabilities[i] > max_prob) {
      next = valid_actions[i];
      max_prob = combined.probabilities[i];
    }
  }
  std::cout << "Next action: " << game.ActionToString(next) << std::endl;

  // Step 5: training-time augmentation — emit `(game, target)` pairs
  // for the replay buffer.
  const TttTrainingAugmenter training_augmenter;
  const TrainingTarget target{combined.value, combined.probabilities};
  const std::vector<std::pair<TttGame, TrainingTarget>> training_pairs =
      training_augmenter.Augment(game, target);
  std::cout << "Number of augmented training data: " << training_pairs.size()
            << std::endl;

  // Smoke-test the in-place transition contract: apply, then undo.
  TttGame working = game;
  const std::size_t round_before = working.CurrentRound();
  working.ApplyActionInPlace(next);
  working.UndoLastAction();
  if (working.CurrentRound() != round_before) {
    std::cerr << "Error: undo did not restore round counter." << std::endl;
    return 1;
  }

  // Step 6: round-trip the same game state through the compact policy
  // serializer / deserializer and verify the recovered probabilities
  // match the input.
  static_assert(
      TttGame::kMaxLegalActions == TttGame::kPolicySize,
      "Tic-tac-toe is dense; kMaxLegalActions should equal kPolicySize.");
  std::array<TttAction, TttGame::kMaxLegalActions> mid_actions{};
  const std::size_t mid_count = game.ValidActionsInto(mid_actions);
  if (mid_count == 0 || mid_count >= TttGame::kPolicySize) {
    std::cerr << "Error: expected a mid-game state with "
                 "0 < legal action count < kPolicySize for the compact test."
              << std::endl;
    return 1;
  }

  std::vector<float> mid_pi;
  mid_pi.reserve(mid_count);
  for (std::size_t i = 0; i < mid_count; ++i) {
    mid_pi.push_back(static_cast<float>(i + 1));
  }
  float mid_pi_sum = 0.0f;
  for (const float v : mid_pi) {
    mid_pi_sum += v;
  }
  for (float& v : mid_pi) {
    v /= mid_pi_sum;
  }
  const TrainingTarget mid_target{0.25f, mid_pi};

  const DefaultCompactPolicyOutputSerializer<TttGame> compact_serializer;
  const CompactPolicyTargetBlob compact_blob =
      compact_serializer.SerializePolicyOutput(game, mid_target);
  if (compact_blob.count != mid_count) {
    std::cerr << "Error: compact serializer count mismatch." << std::endl;
    return 1;
  }
  if (compact_blob.legal_indices.size() != mid_count ||
      compact_blob.values.size() != mid_count) {
    std::cerr << "Error: compact serializer vector sizes mismatch."
              << std::endl;
    return 1;
  }
  for (std::size_t i = 0; i < mid_count; ++i) {
    if (compact_blob.legal_indices[i] != game.PolicyIndex(mid_actions[i])) {
      std::cerr << "Error: compact serializer index mismatch at " << i
                << "." << std::endl;
      return 1;
    }
    if (std::fabs(compact_blob.values[i] - mid_pi[i]) > 1e-6f) {
      std::cerr << "Error: compact serializer value mismatch at " << i
                << "." << std::endl;
      return 1;
    }
  }

  const DefaultCompactPolicyOutputDeserializer<TttGame> compact_deserializer;
  const CompactPolicyOutputBlob compact_output{
      compact_blob.value, std::span<const std::size_t>{compact_blob.legal_indices},
      std::span<const float>{compact_blob.values}};
  const auto compact_decoded =
      compact_deserializer.Deserialize(game, compact_output);
  if (!compact_decoded.has_value()) {
    std::cerr << "Error: compact deserializer rejected its own serializer's "
                 "output: "
              << compact_decoded.error() << std::endl;
    return 1;
  }
  if (compact_decoded->probabilities.size() != mid_count) {
    std::cerr << "Error: compact deserializer probability size mismatch."
              << std::endl;
    return 1;
  }
  if (std::fabs(compact_decoded->value - mid_target.z) > 1e-6f) {
    std::cerr << "Error: compact deserializer value mismatch." << std::endl;
    return 1;
  }
  for (std::size_t i = 0; i < mid_count; ++i) {
    if (std::fabs(compact_decoded->probabilities[i] - mid_pi[i]) > 1e-6f) {
      std::cerr << "Error: compact round-trip mismatch at action " << i
                << "." << std::endl;
      return 1;
    }
  }
  std::cout << "Compact round-trip recovered "
            << compact_decoded->probabilities.size() << " probabilities for "
            << mid_count << " legal actions." << std::endl;

  return 0;
}
