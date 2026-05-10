#ifndef ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_SERIALIZER_H_
#define ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_SERIALIZER_H_

#include <cstddef>
#include <vector>

#include "alpha-zero-api/game.h"
#include "alpha-zero-api/policy_output.h"
#include "alpha-zero-api/serializer.h"

namespace az::game::api {

/**
 * @brief Canonical scatter from a `TrainingTarget` to the network's
 * fixed-size policy output.
 *
 * The output layout is `[z, p_0, p_1, ..., p_{kPolicySize-1}]`, with
 * one float for the value followed by `kPolicySize` floats for the
 * policy. Slots not corresponding to a legal action are left at 0.
 *
 * `target.pi[i]` is written into slot `1 + game.PolicyIndex(actions[i])`,
 * where `actions = game.ValidActions()`. Caller must ensure
 * `target.pi.size() == actions.size()`.
 */
template <Game G>
class DefaultPolicyOutputSerializer : public IPolicyOutputSerializer<G> {
 public:
  DefaultPolicyOutputSerializer() = default;
  ~DefaultPolicyOutputSerializer() override = default;

  [[nodiscard]] std::vector<float> SerializePolicyOutput(
      const G& game, const TrainingTarget& target) const noexcept final {
    std::vector<float> result(G::kPolicySize + 1, 0.0f);
    result[0] = target.z;
    const auto actions = game.ValidActions();
    for (std::size_t i = 0; i < actions.size(); ++i) {
      const std::size_t idx = game.PolicyIndex(actions[i]);
      result[1 + idx] = target.pi[i];
    }
    return result;
  }
};

}  // namespace az::game::api

#endif  // ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_SERIALIZER_H_
