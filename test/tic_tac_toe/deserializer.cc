#include "tic_tac_toe/deserializer.h"

#include <algorithm>
#include <cmath>
#include <expected>
#include <span>
#include <string>
#include <vector>

namespace alphazero::game::api::test {

namespace {

std::vector<float> Softmax(std::span<const float> input) {
  if (input.empty()) return {};

  const float max_val = *std::max_element(input.begin(), input.end());

  std::vector<float> output;
  output.reserve(input.size());
  float sum = 0.0f;
  for (const float val : input) {
    const float exp_val = std::exp(val - max_val);
    output.emplace_back(exp_val);
    sum += exp_val;
  }

  const float inv_sum = 1.0f / sum;
  for (float& val : output) {
    val *= inv_sum;
  }

  return output;
}

}  // namespace

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
  return PolicyOutput{output.front(), std::move(Softmax(probs))};
}

}  // namespace alphazero::game::api::test
