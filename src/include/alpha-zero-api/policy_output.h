#ifndef ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_POLICY_OUTPUT_H_
#define ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_POLICY_OUTPUT_H_

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
 *     `game.ValidActions()[i]`. Length always matches the number of
 *     legal actions on the current state.
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
 *   - `pi[i]` is the MCTS visit-count distribution for the action at
 *     `game.ValidActions()[i]`. The vector sums to 1 and has length
 *     equal to `ValidActions().size()`.
 *
 * Policy-output serializers consume `TrainingTarget`. The fixed-size
 * network output is laid out via `game.PolicyIndex(action)`; see
 * `defaults/serializer.h` for the canonical scatter implementation.
 */
struct TrainingTarget {
  float z;
  std::vector<float> pi;
};

}  // namespace az::game::api

#endif  // ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_POLICY_OUTPUT_H_
