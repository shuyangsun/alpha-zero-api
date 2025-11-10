#ifndef ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_SERIALIZER_H_
#define ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_SERIALIZER_H_

#include <span>
#include <string>
#include <vector>

#include "alpha-zero-api/configuration.h"
#include "alpha-zero-api/game.h"
#include "alpha-zero-api/policy_output.h"

namespace alphazero::game::api {

/**
 * @brief ISerializer is an interface for serializing a particular game state to
 * the neural network input.
 *
 * @tparam B Type of board. See documentation for IGame in
 * src/include/alpha-zero-api/game.h.
 * @tparam A Type of a single action. See documentation for IGame in
 * src/include/alpha-zero-api/game.h.
 * @tparam P Type of player. See documentation for IGame in
 * src/include/alpha-zero-api/game.h.
 */
template <typename B, typename A, typename P>
class IGameSerializer {
 public:
  virtual ~IGameSerializer() = default;

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
};

template <typename B, typename A, typename P>
class IPolicyOutputSerializer {
 public:
  virtual ~IPolicyOutputSerializer() = default;

  /**
   * @brief Serialize a `PolicyOutput` object to a fixed-size vector of floats
   * as the neural network output training data.
   *
   * @param board Current board state.
   * @param player Current player.
   * @param actions Available actions.
   * @param output PolicyOutput object to serialize.
   * @return std::vector<float> Serialized PolicyOutput object as a vector of
   * single-precision floats.
   */
  virtual std::vector<float> Serialize(const B& board, const P& player,
                                       std::span<const A> actions,
                                       const PolicyOutput& output) const = 0;
};

}  // namespace alphazero::game::api

#endif  // ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_SERIALIZER_H_
