#ifndef ALPHA_ZERO_API_API_CPP_ALPHAZEROAPI_INCLUDE_SERIALIZER_H_
#define ALPHA_ZERO_API_API_CPP_ALPHAZEROAPI_INCLUDE_SERIALIZER_H_

#include <span>
#include <string>
#include <vector>

#include "AlphaZeroAPI/include/configuration.h"
#include "AlphaZeroAPI/include/game.h"
#include "AlphaZeroAPI/include/policy_output.h"

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
   * The returned vector should be of fixed size, because it will be used for
   * the neural network input.
   *
   * @param board Current board state.
   * @param player Current player.
   * @return std::vector<float> Serialized neural network input as a vector of
   * floats.
   */
  virtual std::vector<float> Serialize(const B& board, const P& player,
                                       std::span<const A> actions) const = 0;

  /**
   * @brief Serialize a PolicyOutput object to a vector of floats.
   *
   * The returned vector should be of fixed size, because it will be used to
   * provide training data output for the neural network.
   *
   * @param output PolicyOutput object to serialize.
   * @return std::vector<float> Serialized PolicyOutput object as a vector of
   * single-precision floats.
   */
  virtual std::vector<float> Serialize(const PolicyOutput& output) const = 0;

  virtual ~ISerializer() = default;
};

}  // namespace alphazero::game::api

#endif  // ALPHA_ZERO_API_API_CPP_ALPHAZEROAPI_INCLUDE_SERIALIZER_H_
