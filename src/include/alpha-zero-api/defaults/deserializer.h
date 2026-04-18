#ifndef ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_DESERIALIZER_H_
#define ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_DESERIALIZER_H_

#include <string>

#include "alpha-zero-api/deserializer.h"

namespace az::game::api {

/**
 * @brief Default deserializer for neural networks.
 *
 * @tparam B Type of board. See documentation for IGame in
 * src/include/alpha-zero-api/game.h.
 * @tparam A Type of a single action. See documentation for IGame in
 * src/include/alpha-zero-api/game.h.
 * @tparam P Type of player. See documentation for IGame in
 * src/include/alpha-zero-api/game.h.
 */
template <typename B, typename A, typename P>
class DefaultPolicyOutputDeserializer
    : public IPolicyOutputDeserializer<B, A, P, std::string> {
 public:
  DefaultPolicyOutputDeserializer() = default;
  ~DefaultPolicyOutputDeserializer() override = default;

  /**
   * @brief Deserializes the output from the neural network to a PolicyOutput.
   *
   * @param board Current game board.
   * @param player Current player.
   * @param actions Available actions.
   * @param output Neural network output. The first element is the value of the
   * current state, and the rest of the elements are probabilities of selecting
   * each action.
   * @return std::expected<PolicyOutput, std::string> PolicyOutput object if the
   * deserialization is successful, error message otherwise.
   */
  std::expected<PolicyOutput, std::string> Deserialize(
      const B& board, const P& player, std::span<const A> actions,
      std::span<const float> output) const final {
    if (output.size() < 1) {
      return std::unexpected<std::string>(
          "Neural network output size is less than 1.");
    }
    if (output.size() != actions.size() + 1) {
      return std::unexpected<std::string>(std::format(
          "Neural network output size {} does not match the expected size {}.",
          output.size(), actions.size() + 1));
    }
    return PolicyOutput{std::vector<float>{output.begin(), output.end()}};
  }
};

}  // namespace az::game::api

#endif  // ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_DESERIALIZER_H_
