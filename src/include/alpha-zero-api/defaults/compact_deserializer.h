#ifndef ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_COMPACT_DESERIALIZER_H_
#define ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_COMPACT_DESERIALIZER_H_

#include <cstddef>
#include <expected>
#include <string>
#include <utility>
#include <vector>

#include "alpha-zero-api/deserializer.h"
#include "alpha-zero-api/game.h"
#include "alpha-zero-api/policy_output.h"

namespace az::game::api {

/**
 * @brief Canonical gather from a `CompactPolicyOutputBlob` to an
 * `Evaluation`.
 *
 * Reads `output.value` as the value scalar and treats
 * `output.values[j]` as the policy weight for slot
 * `output.legal_indices[j]`. Padding entries
 * (`legal_indices[j] == CompactPolicyTargetBlob::kPaddingSlot`) are
 * skipped. The result's `probabilities[i]` is the weight associated
 * with `game.PolicyIndex(game.ValidActions()[i])`, gathered back into
 * `ValidActions()` order. Returns probabilities verbatim — no implicit
 * softmax or renormalization. Callers whose networks emit logits should
 * compose a softmax in their own deserializer.
 *
 * Returns an error if `output.legal_indices.size() != output.values.size()`,
 * or if any legal action's slot is missing from the (non-padding)
 * entries of `output.legal_indices`.
 *
 * The default uses `std::string` for its error type; games that want
 * typed errors should implement
 * `ICompactPolicyOutputDeserializer<G, MyError>` directly.
 */
template <Game G>
class DefaultCompactPolicyOutputDeserializer
    : public ICompactPolicyOutputDeserializer<G, std::string> {
 public:
  DefaultCompactPolicyOutputDeserializer() = default;
  ~DefaultCompactPolicyOutputDeserializer() override = default;

  [[nodiscard]] std::expected<Evaluation, std::string> Deserialize(
      const G& game,
      const CompactPolicyOutputBlob& output) const noexcept final {
    if (output.legal_indices.size() != output.values.size()) {
      return std::unexpected<std::string>(
          "compact output indices/values size mismatch");
    }
    const auto actions = game.ValidActions();
    std::vector<float> probs(actions.size(), 0.0f);
    for (std::size_t i = 0; i < actions.size(); ++i) {
      const std::size_t slot = game.PolicyIndex(actions[i]);
      bool found = false;
      for (std::size_t j = 0; j < output.legal_indices.size(); ++j) {
        if (output.legal_indices[j] == CompactPolicyTargetBlob::kPaddingSlot) {
          continue;
        }
        if (output.legal_indices[j] == slot) {
          probs[i] = output.values[j];
          found = true;
          break;
        }
      }
      if (!found) {
        return std::unexpected<std::string>(
            "compact output missing slot for a legal action");
      }
    }
    return Evaluation{output.value, std::move(probs)};
  }
};

}  // namespace az::game::api

#endif  // ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_COMPACT_DESERIALIZER_H_
