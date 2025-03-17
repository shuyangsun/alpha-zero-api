#ifndef ALPHA_ZERO_API_API_CPP_INCLUDE_SERIALIZER_H_
#define ALPHA_ZERO_API_API_CPP_INCLUDE_SERIALIZER_H_

#include <string>
#include <vector>

#include "api/cpp/include/game.h"

namespace alphazero::game::api {

/**
 * @brief ISerializer is an interface for serializing a particular game state to
 * the neural network input.
 *
 * @tparam B Type of board. See documentation for IGame in api/cpp/game.h.
 * @tparam A Type of a single action. See documentation for IGame in
 * api/cpp/game.h.
 * @tparam P Type of player. See documentation for IGame in api/cpp/game.h.
 */
template <typename B, typename A, typename P>
class ISerializer {
 public:
  /**
   * @brief Serializes the given game state to a vector of floats as the neural
   * network input. The input format should at least include the current board
   * state, maybe current player or available actions as well. The input format
   * is a very subjective design decision implementations need to make.
   *
   * @param board Current board state.
   * @param player Current player.
   * @return std::vector<float> Serialized neural network input as a vector of
   * floats.
   */
  virtual std::vector<float> Serialize(const B& board, const P& player,
                                       std::span<cons A> actions) const = 0;

  virtual ~ISerializer() = default;
};

}  // namespace alphazero::game::api

#endif  // ALPHA_ZERO_API_API_CPP_INCLUDE_SERIALIZER_H_
