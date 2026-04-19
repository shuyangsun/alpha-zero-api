#ifndef ALPHA_ZERO_API_TEST_TIC_TAC_TOE_SERIALIZER_H_
#define ALPHA_ZERO_API_TEST_TIC_TAC_TOE_SERIALIZER_H_

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "alpha-zero-api/defaults/serializer.h"
#include "alpha-zero-api/serializer.h"
#include "game.h"

namespace az::game::api::test {

class TttSerializer
    : public IGameSerializer<TttBoard, TttAction, TttPlayer>,
      public IPolicyOutputSerializer<TttBoard, TttAction, TttPlayer> {
 public:
  TttSerializer() = default;
  ~TttSerializer() override = default;

  [[nodiscard]] std::vector<float> SerializeCurrentState(
      const TttBoard& board, const TttPlayer& player,
      std::span<const TttAction> actions) const noexcept final;

  [[nodiscard]] std::vector<float> SerializePolicyOutput(
      const TttBoard& board, const TttPlayer& player,
      std::span<const TttAction> actions,
      const PolicyOutput& output) const noexcept final;
};

}  // namespace az::game::api::test

#endif  // ALPHA_ZERO_API_TEST_TIC_TAC_TOE_SERIALIZER_H_
