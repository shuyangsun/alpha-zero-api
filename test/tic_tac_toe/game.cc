#include "tic_tac_toe/game.h"

#include <array>
#include <cctype>
#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace az::game::api::test {

namespace {

static constexpr auto kRowDelim_arr = [] {
  constexpr std::size_t len = 2 + (4 * TTT_COLS + 1) + 1;
  std::array<char, len> arr{};
  arr[0] = ' ';
  arr[1] = ' ';
  for (std::size_t i = 2; i < len - 1; ++i) arr[i] = '-';
  arr[len - 1] = '\n';
  return arr;
}();

constexpr std::string_view kRowDelim{kRowDelim_arr.data(),
                                     kRowDelim_arr.size()};

std::string_view Trim(std::string_view input) {
  while (!input.empty() &&
         std::isspace(static_cast<unsigned char>(input.front()))) {
    input.remove_prefix(1);
  }
  while (!input.empty() &&
         std::isspace(static_cast<unsigned char>(input.back()))) {
    input.remove_suffix(1);
  }
  return input;
}

char CellSymbol(int8_t value) {
  if (value == 1) {
    return 'X';
  }
  if (value == -1) {
    return 'O';
  }
  return ' ';
}

enum class GameStatus : uint8_t {
  ONGOING,
  DRAW,
  WIN_PLAYER_X,
  WIN_PLAYER_O,
};

GameStatus CheckGameStatus(const TttBoard& board) {
  // Check rows and columns
  for (uint16_t i = 0; i < TTT_ROWS; ++i) {
    if (board[i][0] != 0 && board[i][0] == board[i][1] &&
        board[i][1] == board[i][2]) {
      return board[i][0] == 1 ? GameStatus::WIN_PLAYER_X
                              : GameStatus::WIN_PLAYER_O;
    }
    if (board[0][i] != 0 && board[0][i] == board[1][i] &&
        board[1][i] == board[2][i]) {
      return board[0][i] == 1 ? GameStatus::WIN_PLAYER_X
                              : GameStatus::WIN_PLAYER_O;
    }
  }
  // Check diagonals
  if (board[0][0] != 0 && board[0][0] == board[1][1] &&
      board[1][1] == board[2][2]) {
    return board[0][0] == 1 ? GameStatus::WIN_PLAYER_X
                            : GameStatus::WIN_PLAYER_O;
  }
  if (board[0][2] != 0 && board[0][2] == board[1][1] &&
      board[1][1] == board[2][0]) {
    return board[0][2] == 1 ? GameStatus::WIN_PLAYER_X
                            : GameStatus::WIN_PLAYER_O;
  }
  // Check for ongoing or draw
  for (const auto& row : board) {
    for (const auto& cell : row) {
      if (cell == 0) {
        return GameStatus::ONGOING;
      }
    }
  }
  return GameStatus::DRAW;
}

}  // namespace

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
  if (!current_player_) {
    return board_;
  }
  TttBoard result = board_;
  for (auto& row : result) {
    for (auto& cell : row) {
      if (cell == 0) {
        continue;
      }
      cell = -cell;
    }
  }
  return result;
}

std::vector<TttAction> TttGame::ValidActions() const {
  std::vector<TttAction> actions;
  actions.reserve(TTT_ROWS * TTT_COLS);
  for (uint16_t r = 0; r < TTT_ROWS; ++r) {
    for (uint16_t c = 0; c < TTT_COLS; ++c) {
      if (board_[r][c] == 0) {
        actions.push_back(TttAction{r, c});
      }
    }
  }
  return actions;
}

std::unique_ptr<const TttGameInterface> TttGame::GameAfterAction(
    const TttAction& action) const {
  TttGame new_game = *this;
  new_game.board_[action.row][action.col] = current_player_ ? 1 : -1;
  new_game.current_player_ = !current_player_;
  new_game.current_round_++;
  return std::make_unique<TttGame>(std::move(new_game));
}

bool TttGame::IsOver() const {
  return CheckGameStatus(board_) != GameStatus::ONGOING;
}

float TttGame::GetScore(const TttPlayer& player) const {
  using enum GameStatus;
  GameStatus status = CheckGameStatus(board_);
  if (status == ONGOING || status == DRAW) {
    return 0.0f;
  }
  if ((status == WIN_PLAYER_O && !player) ||
      (status == WIN_PLAYER_X && player)) {
    return 1.0f;
  }
  return -1.0f;
}

std::string TttGame::BoardReadableString() const {
  std::ostringstream out;
  out << kRowDelim;
  for (uint16_t r = 0; r < TTT_ROWS; ++r) {
    const uint16_t row_label = static_cast<uint16_t>(TTT_ROWS - r);
    out << row_label << " |";
    for (uint16_t c = 0; c < TTT_COLS; ++c) {
      out << ' ' << CellSymbol(board_[r][c]) << ' ';
      if (c != TTT_COLS - 1) {
        out << '|';
      } else {
        out << "|\n";
      }
    }
    out << kRowDelim;
  }

  out << "    ";
  for (uint16_t c = 0; c < TTT_COLS; ++c) {
    out << static_cast<char>('A' + c);
    if (c != TTT_COLS - 1) {
      out << "   ";
    }
  }
  return out.str();
}

std::expected<TttAction, std::string> TttGame::ActionFromString(
    std::string_view action_str) const {
  action_str = Trim(action_str);
  if (action_str.size() != 2) {
    return std::unexpected(
        std::string{"Action must match pattern <column><row>, e.g. A1"});
  }

  const char col_char_raw = action_str[0];
  const char row_char = action_str[1];

  if (!std::isalpha(static_cast<unsigned char>(col_char_raw))) {
    return std::unexpected(
        std::string{"Column must be a letter between A and C"});
  }
  const char col_char =
      static_cast<char>(std::toupper(static_cast<unsigned char>(col_char_raw)));
  if (col_char < 'A' || col_char >= static_cast<char>('A' + TTT_COLS)) {
    return std::unexpected(std::string{"Column must be between A and C"});
  }

  if (!std::isdigit(static_cast<unsigned char>(row_char))) {
    return std::unexpected(std::string{"Row must be a digit between 1 and 3"});
  }
  const uint16_t row_number = static_cast<uint16_t>(row_char - '0');
  if (row_number < 1 || row_number > TTT_ROWS) {
    return std::unexpected(std::string{"Row must be between 1 and 3"});
  }

  const uint16_t col_index = static_cast<uint16_t>(col_char - 'A');
  const uint16_t row_index = static_cast<uint16_t>(TTT_ROWS - row_number);

  return TttAction{row_index, col_index};
}

std::string TttGame::ActionToString(const TttAction& action) const {
  if (action.row >= TTT_ROWS || action.col >= TTT_COLS) {
    return "Invalid action";
  }

  const char col_char = static_cast<char>('A' + action.col);
  std::stringstream ss;
  ss << col_char << std::to_string(TTT_ROWS - action.row);
  return ss.str();
}

}  // namespace az::game::api::test
