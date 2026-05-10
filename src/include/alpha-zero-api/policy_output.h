#ifndef ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_POLICY_OUTPUT_H_
#define ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_POLICY_OUTPUT_H_

#include <cstddef>
#include <limits>
#include <span>
#include <vector>

namespace az::game::api {

/**
 * @brief What the network produced for a single state.
 *
 * `Evaluation` is the consumer-side decoded form of one forward pass:
 * a scalar value estimate plus a probability distribution over the
 * **currently legal** actions. The distribution is post-softmax,
 * masked to legal actions, and renormalized so it sums to 1.
 *
 *   - `value` is in `[-1, +1]`, from the **current player's**
 *     perspective.
 *   - `probabilities[i]` is the prior for the action at
 *     the i-th action written by `game.ValidActionsInto(...)`. Length
 *     always matches the number of legal actions on the current state.
 *
 * Deserializers produce `Evaluation`. The split between this type and
 * `TrainingTarget` makes the
 * "is-this-the-network-prediction-or-the-training-label?" distinction
 * type-level explicit.
 */
struct Evaluation {
  float value;
  std::vector<float> probabilities;
};

/**
 * @brief What the network is asked to learn for a single state.
 *
 * `TrainingTarget` is the supervised label written to the replay
 * buffer at the end of self-play.
 *
 *   - `z` is the actual game outcome reached in self-play, from the
 *     **current player's** perspective at the time this state was
 *     recorded — produced by `GetScore(state.CurrentPlayer())` once
 *     the playout finishes.
 *   - `pi[i]` is the MCTS visit-count distribution for the i-th action
 *     written by `game.ValidActionsInto(...)`. The vector sums to 1 and
 *     has length equal to the returned legal-action count.
 *
 * Policy-output serializers consume `TrainingTarget`. The fixed-size
 * network output is laid out via `game.PolicyIndex(action)`; see
 * `defaults/serializer.h` for the canonical scatter implementation.
 */
struct TrainingTarget {
  float z;
  std::vector<float> pi;
};

/**
 * @brief Compact policy-target blob written by an
 *        ICompactPolicyOutputSerializer.
 *
 *   - `value` is the scalar value target (same role as
 *     DefaultPolicyOutputSerializer's slot 0).
 *   - `legal_indices[i]` is the slot in [0, kPolicySize) for the
 *     i-th legal action.
 *   - `values[i]` is the policy probability/logit for that slot,
 *     aligned with legal_indices[i].
 *   - All vectors have length `count`. If a fixed-size carrier is
 *     desired (uniform replay-buffer rows), pad to `kMaxLegalActions`
 *     with `legal_indices[i] == kPaddingSlot` and `values[i] == 0`.
 *     Implementations that pad must report the unpadded count via
 *     `count`.
 */
struct CompactPolicyTargetBlob {
  static constexpr std::size_t kPaddingSlot =
      std::numeric_limits<std::size_t>::max();

  float value;
  std::size_t count;
  std::vector<std::size_t> legal_indices;
  std::vector<float> values;
};

/**
 * @brief Compact prediction blob read by an
 *        ICompactPolicyOutputDeserializer. Mirrors the network's output
 *        layout for a compact policy head.
 *
 * The network emits one scalar value plus a length-`kMaxLegalActions`
 * row of probabilities/logits. The engine pairs each row position with
 * the corresponding `legal_indices` from the matching forward-pass
 * input (the engine threads that pairing through the inference path).
 */
struct CompactPolicyOutputBlob {
  float value;
  std::span<const std::size_t> legal_indices;
  std::span<const float> values;
};

}  // namespace az::game::api

#endif  // ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_POLICY_OUTPUT_H_
