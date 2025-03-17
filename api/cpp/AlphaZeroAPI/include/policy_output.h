#ifndef ALPHA_ZERO_API_API_CPP_ALPHAZEROAPI_INCLUDE_POLICY_OUTPUT_H_
#define ALPHA_ZERO_API_API_CPP_ALPHAZEROAPI_INCLUDE_POLICY_OUTPUT_H_

#include <iterator>
#include <utility>
#include <vector>

namespace alphazero::game::api {

/**
 * @brief ** DO NOT MODIFY ** PolicyOutput class is the output of the policy
 * neural network.
 *
 * It contains the value of the current state and the vector of
 * probabilities of selecting each action. The value and probabilities are
 * single-precision floating point values.
 */
struct PolicyOutput {
  PolicyOutput() = delete;

  /**
   * @brief Construct a new PolicyOutput object from a value and an r-value
   * vector of probabilities.
   *
   * @param value Value of the current state.
   * @param probabilities Vector of probabilities of selecting each action.
   */
  PolicyOutput(float value, std::vector<float>&& probabilities)
      : value{value}, probabilities{std::move(probabilities)} {}

  /**
   * @brief Construct a new PolicyOutput object from a value and an l-value
   * vector of probabilities.
   *
   * @param value Value of the current state.
   * @param probabilities Vector of probabilities of selecting each action.
   */
  PolicyOutput(float value, const std::vector<float>& probabilities)
      : value{value}, probabilities{probabilities} {}

  /**
   * @brief Construct a new PolicyOutput object from an r-value vector that
   * represents the output of the neural network. The first element of the
   * vector is the value of the current state, and the rest of the elements are
   * probabilities of selecting each action.
   *
   * @param nn_output Vector of single-precision floats that represents the
   * output of the neural network.
   */
  PolicyOutput(std::vector<float>&& nn_output)
      : value{nn_output.front()},
        probabilities{std::make_move_iterator(nn_output.begin() + 1),
                      std::make_move_iterator(nn_output.end())} {}

  /**
   * @brief Construct a new PolicyOutput object from an r-value vector that
   * represents the output of the neural network. The first element of the
   * vector is the value of the current state, and the rest of the elements are
   * probabilities of selecting each action.
   *
   * @param nn_output Vector of single-precision floats that represents the
   * output of the neural network.
   */
  PolicyOutput(const std::vector<float>& nn_output)
      : value{nn_output.front()},
        probabilities{nn_output.begin() + 1, nn_output.end()} {}

  ~PolicyOutput() = default;

  /**
   * @brief Value of the current state.
   */
  float value;

  /**
   * @brief Probablities of selecting each action.
   */
  std::vector<float> probabilities;
};

}  // namespace alphazero::game::api

#endif  // ALPHA_ZERO_API_API_CPP_ALPHAZEROAPI_INCLUDE_POLICY_OUTPUT_H_
