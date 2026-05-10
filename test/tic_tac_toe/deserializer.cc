#include "tic_tac_toe/deserializer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <expected>
#include <span>
#include <utility>
#include <vector>

#include "alpha-zero-api/policy_output.h"
#include "tic_tac_toe/game.h"

namespace az::game::api::test {

namespace {

std::vector<float> Softmax(std::span<const float> input) noexcept {
  if (input.empty()) {
    return {};
  }

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

TttResult<Evaluation> TttDeserializer::Deserialize(
    const TttGame& game, std::span<const float> output) const noexcept {
  if (output.size() != TttGame::kPolicySize + 1) {
    return std::unexpected(TttError::kInvalidPolicyOutputSize);
  }
  std::array<TttAction, TttGame::kMaxLegalActions> actions{};
  const std::size_t count = game.ValidActionsInto(actions);
  std::vector<float> raw;
  raw.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    const std::size_t idx = game.PolicyIndex(actions[i]);
    raw.push_back(output[1 + idx]);
  }
  return Evaluation{output.front(), Softmax(raw)};
}

}  // namespace az::game::api::test
