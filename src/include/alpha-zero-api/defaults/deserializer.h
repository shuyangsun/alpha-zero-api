#ifndef ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_DESERIALIZER_H_
#define ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_DESERIALIZER_H_

#include <string>

#include "alpha-zero-api/deserializer.h"

namespace alphazero::game::api {

template <typename B, typename A, typename P>
class DefaultPolicyOutputDeserializer
    : public IPolicyOutputDeserializer<B, A, P, std::string> {
 public:
  DefaultPolicyOutputDeserializer() = default;
  ~DefaultPolicyOutputDeserializer() override = default;

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

}  // namespace alphazero::game::api

#endif  // ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_DESERIALIZER_H_
