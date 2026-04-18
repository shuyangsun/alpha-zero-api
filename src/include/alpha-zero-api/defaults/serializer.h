#ifndef ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_SERIALIZER_H_
#define ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_SERIALIZER_H_

#include "alpha-zero-api/serializer.h"

namespace az::game::api {

template <typename B, typename A, typename P>
class DefaultPolicyOutputSerializer : public IPolicyOutputSerializer<B, A, P> {
 public:
  DefaultPolicyOutputSerializer() = default;
  ~DefaultPolicyOutputSerializer() override = default;

  std::vector<float> SerializePolicyOutput(
      const B& board, const P& player, std::span<const A> actions,
      const PolicyOutput& output) const final {
    std::vector<float> result;
    result.reserve(output.probabilities.size() + 1);
    result.emplace_back(output.value);
    result.insert(result.end(), output.probabilities.begin(),
                  output.probabilities.end());
    return result;
  }
};

}  // namespace az::game::api

#endif  // ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_DEFAULTS_SERIALIZER_H_
