#ifndef ALPHA_ZERO_API_TEST_TIC_TAC_TOE_DESERIALIZER_H_
#define ALPHA_ZERO_API_TEST_TIC_TAC_TOE_DESERIALIZER_H_

#include <expected>
#include <span>

#include "alpha-zero-api/deserializer.h"
#include "alpha-zero-api/policy_output.h"
#include "tic_tac_toe/game.h"

namespace az::game::api::test {

/**
 * @brief Decode a `[value, p_0, ..., p_8]` neural-network output for
 * `TttGame`.
 *
 * The probabilities sub-vector is masked to the legal action subset
 * (read at `1 + game.PolicyIndex(action)`) and softmaxed before being
 * returned as the `Evaluation`'s `probabilities`. Returns
 * `kInvalidPolicyOutputSize` if the input does not have exactly
 * `kPolicySize + 1` elements.
 */
class TttDeserializer : public IPolicyOutputDeserializer<TttGame, TttError> {
 public:
  TttDeserializer() = default;
  ~TttDeserializer() override = default;

  [[nodiscard]] TttResult<Evaluation> Deserialize(
      const TttGame& game, std::span<const float> output) const noexcept final;
};

}  // namespace az::game::api::test

#endif  // ALPHA_ZERO_API_TEST_TIC_TAC_TOE_DESERIALIZER_H_
