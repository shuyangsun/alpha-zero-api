#ifndef ALPHA_ZERO_API_TEST_TIC_TAC_TOE_SERIALIZER_H_
#define ALPHA_ZERO_API_TEST_TIC_TAC_TOE_SERIALIZER_H_

#include <vector>

#include "alpha-zero-api/ring_buffer.h"
#include "alpha-zero-api/serializer.h"
#include "tic_tac_toe/game.h"

namespace az::game::api::test {

/**
 * @brief Serializes a `TttGame` board into a 9-element neural-network
 * input.
 *
 * Tic-Tac-Toe is Markov, so the engine-supplied history view is
 * always empty and ignored. The board is flattened in row-major
 * order; the current player's pieces appear as `+1` and the
 * opponent's as `-1`, matching the canonical AlphaZero input
 * convention.
 */
class TttSerializer : public IGameSerializer<TttGame> {
 public:
  TttSerializer() = default;
  ~TttSerializer() override = default;

  [[nodiscard]] std::vector<float> SerializeCurrentState(
      const TttGame& game,
      RingBufferView<TttGame> history) const noexcept final;
};

}  // namespace az::game::api::test

#endif  // ALPHA_ZERO_API_TEST_TIC_TAC_TOE_SERIALIZER_H_
