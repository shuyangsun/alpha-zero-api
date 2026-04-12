#ifndef ALPHA_ZERO_API_TEST_TIC_TAC_TOE_DESERIALIZER_H_
#define ALPHA_ZERO_API_TEST_TIC_TAC_TOE_DESERIALIZER_H_

#include <expected>
#include <span>
#include <string>

#include "alpha-zero-api/deserializer.h"
#include "alpha-zero-api/policy_output.h"
#include "game.h"

namespace alphazero::game::api::test {

class TttDeserializer
    : public IPolicyOutputDeserializer<TttBoard, TttAction, TttPlayer,
                                       std::string> {
 public:
  TttDeserializer() = default;
  ~TttDeserializer() override = default;

  std::expected<PolicyOutput, std::string> Deserialize(
      const TttBoard& board, const TttPlayer& player,
      std::span<const TttAction> actions,
      std::span<const float> output) const final;
};

}  // namespace alphazero::game::api::test

#endif  // ALPHA_ZERO_API_TEST_TIC_TAC_TOE_DESERIALIZER_H_
