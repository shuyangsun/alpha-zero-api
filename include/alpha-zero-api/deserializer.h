#ifndef ALPHA_ZERO_API_INCLUDE_ALPHA_ZERO_API_DESERIALIZER_H_
#define ALPHA_ZERO_API_INCLUDE_ALPHA_ZERO_API_DESERIALIZER_H_

#include <expected>
#include <span>

#include "alpha-zero-api/game.h"
#include "alpha-zero-api/policy_output.h"

namespace alphazero::game::api {

/**
 * @brief Deserializes output from the neural network to PolicyOutput object.
 *
 * TODO: explain when to use default when to use customized.
 *
 * @tparam A Type of a single action. See documentation for IGame in
 * api/cpp/game.h.
 */
template <typename A>
class IDeserializer {
 public:
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
   * @return std::expected<PolicyOutput, std::string> PolicyOutput object if the
   * deserialization is successful, error message otherwise.
   */
  virtual std::expected<PolicyOutput, std::string> Deserialize(
      std::span<const A> actions, std::span<const float> output) const = 0;

  virtual ~IDeserializer() = default;
};

}  // namespace alphazero::game::api

#endif  // ALPHA_ZERO_API_INCLUDE_ALPHA_ZERO_API_DESERIALIZER_H_
