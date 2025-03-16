#include "api/policy_output.h"

#include <iterator>
#include <utility>
#include <vector>

namespace alphazero::api {

PolicyOutput::PolicyOutput(float value, std::vector<float>&& probabilities)
    : value{value}, probabilities{std::move(probabilities)} {}

PolicyOutput::PolicyOutput(float value, const std::vector<float>& probabilities)
    : value{value}, probabilities{probabilities} {}

PolicyOutput::PolicyOutput(std::vector<float>&& nn_output)
    : value{nn_output.front()},
      probabilities{std::make_move_iterator(nn_output.begin() + 1),
                    std::make_move_iterator(nn_output.end())} {}

PolicyOutput::PolicyOutput(const std::vector<float>& nn_output)
    : value{nn_output.front()},
      probabilities{nn_output.begin() + 1, nn_output.end()} {}

}  // namespace alphazero::api
