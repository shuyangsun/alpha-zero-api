#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <span>
#include <utility>
#include <vector>

#include "alpha-zero-api/defaults/serializer.h"
#include "alpha-zero-api/policy_output.h"
#include "alpha-zero-api/ring_buffer.h"
#include "tic_tac_toe/deserializer.h"
#include "tic_tac_toe/game.h"
#include "tic_tac_toe/inference.h"
#include "tic_tac_toe/serializer.h"
#include "tic_tac_toe/train.h"

namespace {

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
    const std::vector<TttAction> aug_actions = aug_game.ValidActions();
    const TrainingTarget fake_target{
        0.5f,
        std::vector<float>(aug_actions.size(),
                           1.0f / static_cast<float>(aug_actions.size()))};
    const std::vector<float> nn_output =
        policy_serializer.SerializePolicyOutput(aug_game, fake_target);
    const TttResult<Evaluation> decoded =
        deserializer.Deserialize(aug_game, nn_output);
    if (!decoded.has_value()) {
      std::cerr << "Error: deserializer rejected its own serializer's output."
                << std::endl;
      return 1;
    }
    assert(decoded->probabilities.size() == aug_actions.size());
    augmented_evaluations.push_back(*decoded);
  }
  std::cout << "Serialized policy output vector length: "
            << TttGame::kPolicySize + 1 << std::endl;

  // Step 4: combine the per-augmentation evaluations back into one for
  // the original game.
  const Evaluation combined =
      inference.Interpret(game, std::span<const TttGame>{augmented_games},
                          std::span<const Evaluation>{augmented_evaluations});

  const std::vector<TttAction> valid_actions = game.ValidActions();
  TttAction next = TttAction{0, 0};
  float max_prob = -1.0f;
  for (std::size_t i = 0; i < valid_actions.size(); ++i) {
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

  return 0;
}
