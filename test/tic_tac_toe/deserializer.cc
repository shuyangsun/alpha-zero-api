#include "tic_tac_toe/deserializer.h"

#include <expected>
#include <span>
#include <string>

namespace alphazero::game::api::test {

std::expected<PolicyOutput, std::string> TttDeserializer::Deserialize(
    const TttBoard& board, const TttPlayer& player,
    std::span<const TttAction> actions, std::span<const float> output) const {
  if (output.size() != TTT_ROWS * TTT_COLS + 1) {
    return std::unexpected<std::string>(
        "Neural network output size is not equal to board size + 1.");
  }
  std::vector<float> probs;
  probs.reserve(actions.size());

  for (const TttAction& action : actions) {
    const size_t index = action.row * TTT_COLS + action.col;
    probs.emplace_back(output[index + 1]);
  }
  // TODO: should we use softmax to ensure probabilities sum to 1?
  return PolicyOutput{output.front(), std::move(probs)};
}

}  // namespace alphazero::game::api::test
