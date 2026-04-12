#ifndef ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DESERIALIZER_H_
#define ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DESERIALIZER_H_

#include <expected>
#include <format>
#include <span>

#include "alpha-zero-api/game.h"
#include "alpha-zero-api/policy_output.h"

namespace alphazero::game::api {

/**
 * @brief Deserializes output from the neural network to a PolicyOutput object.
 *
 * TODO: explain when to use default when to use customized.
 *
 * @tparam B Type of board. See documentation for IGame in
 * src/include/alpha-zero-api/game.h.
 * @tparam A Type of a single action. See documentation for IGame in
 * src/include/alpha-zero-api/game.h.
 * @tparam P Type of player. See documentation for IGame in
 * src/include/alpha-zero-api/game.h.
 * @tparam E Type of error message when deserialization fails.
 */
template <typename B, typename A, typename P, typename E>
class IPolicyOutputDeserializer {
 public:
  virtual ~IPolicyOutputDeserializer() = default;

  /**
   * @brief Deserializes the output from the neural network to a PolicyOutput.
   *
   * If the neural network output format is unexpected, return an error message.
   * The PolicyOutput object's probabilities vector should match the legnth of
   * the actions vector. If the length does not match, tests will fail and the
   * library will throw an error.
   *
   * @param actions Actions passed in to the serializer to generated the neural
   * network input that was used to generate this output.
   * @param output Output of the neural network.
   * @return std::expected<PolicyOutput, E> PolicyOutput object if the
   * deserialization is successful, error type otherwise.
   */
  virtual std::expected<PolicyOutput, E> Deserialize(
      const B& board, const P& player, std::span<const A> actions,
      std::span<const float> output) const = 0;
};

}  // namespace alphazero::game::api

#endif  // ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DESERIALIZER_H_
