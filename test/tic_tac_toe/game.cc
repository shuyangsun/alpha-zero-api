#include "tic_tac_toe/game.h"

#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace alphazero::game::api::test {

TttGame::TttGame(TttPlayer starting_player)
    : current_player_(starting_player) {}

std::unique_ptr<const TttGameInterface> TttGame::Copy() const {
  return std::make_unique<TttGame>(*this);
}

const TttBoard& TttGame::GetBoard() const { return board_; }

uint32_t TttGame::CurrentRound() const { return current_round_; }

TttPlayer TttGame::CurrentPlayer() const { return current_player_; }

std::optional<TttPlayer> TttGame::LastPlayer() const {
  return last_action_.has_value() ? std::optional<TttPlayer>{!current_player_}
                                  : std::nullopt;
}

std::optional<TttAction> TttGame::LastAction() const { return last_action_; }

TttBoard TttGame::CanonicalBoard() const {
  // TODO: Return the canonical board from the current player's view.
  return board_;
}

std::vector<TttAction> TttGame::ValidActions() const {
  // TODO: Calculate the valid actions for the current player.
  return {};
}

std::unique_ptr<const TttGameInterface> TttGame::GameAfterAction(
    const TttAction& /*action*/) const {
  // TODO: Produce the game state that follows the given action.
  return nullptr;
}

bool TttGame::IsOver() const {
  // TODO: Determine whether the Tic Tac Toe game has ended.
  return false;
}

float TttGame::GetScore(const TttPlayer& /*player*/) const {
  // TODO: Compute the final score for the provided player.
  return 0.0f;
}

std::string TttGame::BoardReadableString() const {
  // TODO: Produce a human-readable representation of the board.
  return "Board rendering not implemented";
}

std::expected<TttAction, std::string> TttGame::ActionFromString(
    std::string_view /*action_str*/) const {
  // TODO: Parse an action from its string representation.
  return std::unexpected(std::string{"Action parsing not implemented"});
}

std::string TttGame::ActionToString(const TttAction& /*action*/) const {
  // TODO: Convert an action to its string representation.
  return "Action string conversion not implemented";
}

}  // namespace alphazero::game::api::test
