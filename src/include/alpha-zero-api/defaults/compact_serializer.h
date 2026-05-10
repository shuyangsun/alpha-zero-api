#ifndef ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_COMPACT_SERIALIZER_H_
#define ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_COMPACT_SERIALIZER_H_

#include <array>
#include <cstddef>

#include "alpha-zero-api/game.h"
#include "alpha-zero-api/policy_output.h"
#include "alpha-zero-api/serializer.h"

namespace az::game::api {

/**
 * @brief Canonical pack from a `TrainingTarget` to a
 * `CompactPolicyTargetBlob`.
 *
 * Produces a blob whose `legal_indices[i] = game.PolicyIndex(actions[i])`
 * and `values[i] = target.pi[i]`, where `actions[0..count)` is filled by
 * `game.ValidActionsInto(actions)`. `value` carries `target.z`. No padding
 * is applied; downstream code that wants fixed-width rows pads to
 * `G::kMaxLegalActions` and reports the unpadded count via
 * `CompactPolicyTargetBlob::count`.
 *
 * Caller must ensure `target.pi.size() == actions.size()`.
 */
template <Game G>
class DefaultCompactPolicyOutputSerializer
    : public ICompactPolicyOutputSerializer<G> {
 public:
  DefaultCompactPolicyOutputSerializer() = default;
  ~DefaultCompactPolicyOutputSerializer() override = default;

  [[nodiscard]] CompactPolicyTargetBlob SerializePolicyOutput(
      const G& game, const TrainingTarget& target) const noexcept final {
    static_assert(G::kMaxLegalActions <= G::kPolicySize,
                  "kMaxLegalActions must not exceed kPolicySize");
    std::array<typename G::action_t, G::kMaxLegalActions> actions{};
    const std::size_t count = game.ValidActionsInto(actions);
    CompactPolicyTargetBlob blob;
    blob.value = target.z;
    blob.count = count;
    blob.legal_indices.reserve(count);
    blob.values.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
      blob.legal_indices.push_back(game.PolicyIndex(actions[i]));
      blob.values.push_back(target.pi[i]);
    }
    return blob;
  }
};

}  // namespace az::game::api

#endif  // ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_COMPACT_SERIALIZER_H_
