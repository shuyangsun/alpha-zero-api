#ifndef ALPHA_ZERO_API_TEST_TIC_TAC_TOE_SERIALIZER_H_
#define ALPHA_ZERO_API_TEST_TIC_TAC_TOE_SERIALIZER_H_

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "alpha-zero-api/defaults/serializer.h"
#include "alpha-zero-api/serializer.h"
#include "game.h"

namespace alphazero::game::api::test {

class TttSerializer
    : public IGameSerializer<TttBoard, TttAction, TttPlayer>,
      public IPolicyOutputSerializer<TttBoard, TttAction, TttPlayer> {
 public:
  TttSerializer() = default;
  ~TttSerializer() override = default;

  std::vector<float> SerializeCurrentState(
      const TttBoard& board, const TttPlayer& player,
      std::span<const TttAction> actions) const final;

  std::vector<float> SerializePolicyOutput(
      const TttBoard& board, const TttPlayer& player,
      std::span<const TttAction> actions,
      const PolicyOutput& output) const final;
};

}  // namespace alphazero::game::api::test

#endif  // ALPHA_ZERO_API_TEST_TIC_TAC_TOE_SERIALIZER_H_
