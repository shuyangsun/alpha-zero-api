#ifndef ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_SERIALIZER_H_
#define ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_SERIALIZER_H_

#include <vector>

#include "alpha-zero-api/configuration.h"
#include "alpha-zero-api/game.h"
#include "alpha-zero-api/policy_output.h"
#include "alpha-zero-api/ring_buffer.h"

namespace az::game::api {

/**
 * @brief Serialize a game state to the neural network's input layout.
 *
 * `history` is the engine-owned window of past game states; the
 * engine guarantees `history.Size() == G::kHistoryLookback` once
 * enough states have been seen and a shorter prefix during the first
 * few plies of a game. Markov games declaring `kHistoryLookback == 0`
 * always see an empty view.
 *
 * Index 0 of the view is the most recent state preceding `game`;
 * index `Size() - 1` is the oldest state still in the window. `game`
 * itself is **not** in the view.
 */
template <Game G>
class IGameSerializer {
 public:
  virtual ~IGameSerializer() = default;

  [[nodiscard]] virtual std::vector<float> SerializeCurrentState(
      const G& game, RingBufferView<G> history) const noexcept = 0;
};

/**
 * @brief Serialize a `TrainingTarget` to the neural network's output
 * layout.
 *
 * `target.pi[i]` corresponds to `game.ValidActions()[i]`; the
 * implementation is responsible for scattering those values into the
 * fixed-size policy output via `game.PolicyIndex(action)`. The
 * resulting vector typically has size `G::kPolicySize + value_dim` —
 * the canonical default in `defaults/serializer.h` produces
 * `1 + kPolicySize` floats with `target.z` in slot 0.
 */
template <Game G>
class IPolicyOutputSerializer {
 public:
  virtual ~IPolicyOutputSerializer() = default;

  [[nodiscard]] virtual std::vector<float> SerializePolicyOutput(
      const G& game, const TrainingTarget& target) const noexcept = 0;
};

}  // namespace az::game::api

#endif  // ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_SERIALIZER_H_
