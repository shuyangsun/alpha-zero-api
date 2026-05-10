#ifndef ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_DESERIALIZER_H_
#define ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_DESERIALIZER_H_

#include <cstddef>
#include <expected>
#include <format>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "alpha-zero-api/deserializer.h"
#include "alpha-zero-api/game.h"
#include "alpha-zero-api/policy_output.h"

namespace az::game::api {

/**
 * @brief Canonical gather from a network output to an `Evaluation`.
 *
 * Expects `output.size() == G::kPolicySize + 1`, with `output[0]`
 * carrying the value scalar and `output[1 + i]` carrying the prior
 * for policy slot `i`. Reads the masked subset corresponding to
 * `game.ValidActions()` via `game.PolicyIndex(action)`, and returns
 * those probabilities verbatim — no implicit softmax or
 * renormalization. Callers whose networks emit logits should compose
 * a softmax in their own deserializer.
 *
 * The default uses `std::string` for its error type; games that want
 * typed errors should implement `IPolicyOutputDeserializer<G, Error>`
 * directly.
 */
template <Game G>
class DefaultPolicyOutputDeserializer
    : public IPolicyOutputDeserializer<G, std::string> {
 public:
  DefaultPolicyOutputDeserializer() = default;
  ~DefaultPolicyOutputDeserializer() override = default;

  [[nodiscard]] std::expected<Evaluation, std::string> Deserialize(
      const G& game, std::span<const float> output) const noexcept final {
    constexpr std::size_t expected_size = G::kPolicySize + 1;
    if (output.size() != expected_size) {
      return std::unexpected<std::string>(std::format(
          "Neural network output size {} does not match the expected size {}.",
          output.size(), expected_size));
    }
    const auto actions = game.ValidActions();
    std::vector<float> probs;
    probs.reserve(actions.size());
    for (const auto& a : actions) {
      probs.push_back(output[1 + game.PolicyIndex(a)]);
    }
    return Evaluation{output.front(), std::move(probs)};
  }
};

}  // namespace az::game::api

#endif  // ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_DESERIALIZER_H_
