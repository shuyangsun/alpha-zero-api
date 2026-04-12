#include <expected>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "alpha-zero-api/defaults/deserializer.h"
#include "alpha-zero-api/policy_output.h"
#include "tic_tac_toe/game.h"
#include "tic_tac_toe/serializer.h"

namespace {

using ::alphazero::game::api::DefaultPolicyOutputDeserializer;
using ::alphazero::game::api::PolicyOutput;

using ::alphazero::game::api::test::TttAction;
using ::alphazero::game::api::test::TttBoard;
using ::alphazero::game::api::test::TttGame;
using ::alphazero::game::api::test::TttPlayer;
using ::alphazero::game::api::test::TttSerializer;

}  // namespace

int main() {
  const auto game = std::make_unique<TttGame>()
                        ->GameAfterAction(TttAction{1, 1})
                        ->GameAfterAction(TttAction{0, 1})
                        ->GameAfterAction(TttAction{2, 0});
  std::cout << "Current board:\n" << game->BoardReadableString() << "\n";

  const auto serializer = std::make_unique<TttSerializer>();
  const std::vector<float> state = serializer->SerializeCurrentState(
      game->GetBoard(), game->CurrentPlayer(), game->ValidActions());
  std::cout << "Serialied state vector length: " << state.size() << std::endl;

  // Simulate one round of MCTS.
  const PolicyOutput output{
      0.5f, std::vector<float>(game->ValidActions().size(), 0.1f)};

  const std::vector<float> output_vec = serializer->SerializePolicyOutput(
      game->GetBoard(), game->CurrentPlayer(), game->ValidActions(), output);
  std::cout << "Serialied policy output vector length: " << output_vec.size()
            << std::endl;

  const auto deserialier = std::make_unique<
      DefaultPolicyOutputDeserializer<TttBoard, TttAction, TttPlayer>>();
  const std::expected<PolicyOutput, std::string> deserialized_output =
      deserialier->Deserialize(game->GetBoard(), game->CurrentPlayer(),
                               game->ValidActions(), output_vec);
  std::cout << "Deserialization "
            << (deserialized_output.has_value() ? "succeeded." : "failed.")
            << std::endl;

  return 0;
}
