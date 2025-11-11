#ifndef ALPHA_ZERO_API_TEST_TIC_TAC_TOE_GAME_H_
#define ALPHA_ZERO_API_TEST_TIC_TAC_TOE_GAME_H_

#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "alpha-zero-api/defaults/game.h"

namespace alphazero::game::api::test {

constexpr uint16_t TTT_ROWS = 3;
constexpr uint16_t TTT_COLS = 3;
using TttBoard = Standard2DBoard<TTT_ROWS, TTT_COLS>;
using TttAction = Action2D;
using TttPlayer = BinaryPlayer;
using TttGameInterface = IGame<TttBoard, TttAction, TttPlayer>;

class TttGame : public TttGameInterface {
 public:
  TttGame(TttPlayer starting_player = false);
  TttGame(const TttGame& other) = default;

  ~TttGame() override = default;

  std::unique_ptr<const TttGameInterface> Copy() const final;
  const TttBoard& GetBoard() const final;
  uint32_t CurrentRound() const final;
  TttPlayer CurrentPlayer() const final;
  std::optional<TttPlayer> LastPlayer() const final;
  std::optional<TttAction> LastAction() const final;
  TttBoard CanonicalBoard() const final;

  // Actions
  std::vector<TttAction> ValidActions() const final;
  std::unique_ptr<const TttGameInterface> GameAfterAction(
      const TttAction& action) const final;
  bool IsOver() const final;
  float GetScore(const TttPlayer& player) const final;

  // String conversions
  std::string BoardReadableString() const final;
  std::expected<TttAction, std::string> ActionFromString(
      std::string_view action_str) const final;
  std::string ActionToString(const TttAction& action) const final;

 private:
  TttBoard board_ = TttBoard{};
  uint32_t current_round_ = 0;
  TttPlayer current_player_;
  std::optional<TttAction> last_action_ = std::nullopt;
};

}  // namespace alphazero::game::api::test

#endif  // ALPHA_ZERO_API_TEST_TIC_TAC_TOE_GAME_H_
