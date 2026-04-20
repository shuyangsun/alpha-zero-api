#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "alpha-zero-api/policy_output.h"
#include "tic_tac_toe/deserializer.h"
#include "tic_tac_toe/game.h"
#include "tic_tac_toe/inference.h"
#include "tic_tac_toe/serializer.h"
#include "tic_tac_toe/train.h"

namespace {

using ::az::game::api::PolicyOutput;

using ::az::game::api::test::TttAction;
using ::az::game::api::test::TttBoard;
using ::az::game::api::test::TttDeserializer;
using ::az::game::api::test::TttError;
using ::az::game::api::test::TttGame;
using ::az::game::api::test::TttGamePtr;
using ::az::game::api::test::TttInferenceAugmenter;
using ::az::game::api::test::TttPlayer;
using ::az::game::api::test::TttResult;
using ::az::game::api::test::TttSerializer;
using ::az::game::api::test::TttTrainingAugmenter;

}  // namespace

int main() {
  TttResult<TttGamePtr> game_result = TttGame::Create();
  if (!game_result.has_value()) {
    std::cerr << "Error creating game: "
              << static_cast<std::underlying_type_t<TttError>>(
                     game_result.error())
              << std::endl;
    return 1;
  }
  const TttGamePtr game = std::move(*game_result)
                              ->GameAfterAction(TttAction{1, 1})
                              ->GameAfterAction(TttAction{0, 1})
                              ->GameAfterAction(TttAction{2, 0});
  std::cout << "Current board:\n" << game->BoardReadableString() << "\n";

  // Step 1 (optional): augment game state for inference.
  std::vector<TttAction> valid_actions = game->ValidActions();
  const auto inference = std::make_unique<TttInferenceAugmenter>();
  std::unordered_map<uint8_t,
                     std::tuple<TttBoard, TttPlayer, std::vector<TttAction>>>
      augmented_games = inference->Augment(
          game->GetBoard(), game->CurrentPlayer(), valid_actions);
  if (augmented_games.empty()) {
    std::cerr << "Error: Augmented game state vector size is empty."
              << std::endl;
    return 1;
  }
  std::cout << "Number of augmented games: " << augmented_games.size()
            << std::endl;

  // Step 2: serialize augmented game states.
  std::vector<std::vector<float>> augmented_game_states;
  augmented_game_states.reserve(augmented_games.size());
  const auto serializer = std::make_unique<TttSerializer>();
  for (const auto& [key, value] : augmented_games) {
    const auto& [board, player, actions] = value;
    augmented_game_states.emplace_back(
        serializer->SerializeCurrentState(board, player, actions));
    if (augmented_game_states.size() > 1 &&
        augmented_game_states.back().size() !=
            augmented_game_states.front().size()) {
      std::cerr << "Error: Augmented game state vector size is not the "
                   "same across augmented games."
                << std::endl;
      return 1;
    }
  }
  std::cout << "Serialied state vector length: "
            << augmented_game_states.front().size() << std::endl;

  // Step 3: simulate one round of MCTS.
  std::unordered_map<uint8_t, std::vector<float>> aug_outputs;
  for (const auto& [key, value] : augmented_games) {
    const auto& [board, player, actions] = value;
    const PolicyOutput output{
        0.5f, std::vector<float>(actions.size(), 1.0f / actions.size())};
    const std::vector<float> output_vec =
        serializer->SerializePolicyOutput(board, player, actions, output);
    aug_outputs.emplace(key, output_vec);
  }
  std::cout << "Serialied policy output vector length: "
            << aug_outputs.begin()->second.size() << std::endl;

  // Step 4: deserialize the policy output to get the value and probabilities
  // for the original game state.
  std::unordered_map<uint8_t, PolicyOutput> deserialized_aug_outputs;
  deserialized_aug_outputs.reserve(aug_outputs.size());
  for (const auto& [key, value] : aug_outputs) {
    const auto deserialier = std::make_unique<TttDeserializer>();
    const TttResult<PolicyOutput> deserialized_output =
        deserialier->Deserialize(game->GetBoard(), game->CurrentPlayer(),
                                 game->ValidActions(), value);
    assert(deserialized_output.has_value());
    assert(deserialized_output->probabilities.size() == valid_actions.size());
    deserialized_aug_outputs.emplace(key, *deserialized_output);
  }

  // Step 5: interpret multiple policy outputs from augmented games to get the
  // final policy output for the original game state.
  PolicyOutput output =
      inference->Interpret(augmented_games, deserialized_aug_outputs);

  TttAction next = TttAction{0, 0};
  float max_prob = -1.0f;
  for (size_t i = 0; i < valid_actions.size(); ++i) {
    if (output.probabilities[i] > max_prob) {
      next = valid_actions[i];
      max_prob = output.probabilities[i];
    }
  }

  std::cout << "Next action: " << game->ActionToString(next) << std::endl;

  const auto training_augmenter = std::make_unique<TttTrainingAugmenter>();
  const std::vector<
      std::tuple<TttBoard, TttPlayer, std::vector<TttAction>, PolicyOutput>>
      augmented_training_data =
          training_augmenter->Augment(game->GetBoard(), game->CurrentPlayer(),
                                      game->ValidActions(), std::move(output));
  std::cout << "Number of augmented training data: "
            << augmented_training_data.size() << std::endl;

  return 0;
}
