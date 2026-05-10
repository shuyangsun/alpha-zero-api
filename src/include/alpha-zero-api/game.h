#ifndef ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_GAME_H_
#define ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_GAME_H_

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace az::game::api {

/**
 * @brief Concept describing a concrete game implementation.
 *
 * Replaces the older virtual `IGame<B, A, P, E>` interface with a
 * value-semantic, statically-dispatched contract. Concrete games are
 * plain value types satisfying this concept; the engine is templated
 * on the concrete game type. This unblocks stack allocation, removes
 * virtual-dispatch overhead on the MCTS hot path, and lets the
 * compiler see `sizeof(G)`.
 *
 * A conforming `G` must expose:
 *
 *   - Associated types `board_t`, `action_t`, `player_t`, `error_t`.
 *   - A static `kHistoryLookback` declaring how many past states the
 *     game's serializer needs as input. Markov games declare 0.
 *   - A static `kPolicySize` declaring the size of the network's
 *     fixed-size policy head — the cardinality of the game's full
 *     action space, ignoring legality. The bijection between an
 *     `action_t` value and a slot in `[0, kPolicySize)` is given by
 *     `PolicyIndex(a)`.
 *   - A static `kMaxRounds` declaring an optional self-play hard cap.
 *     If set, `IsOver()` must return true once
 *     `CurrentRound() >= *kMaxRounds`. The cap exists so pathological
 *     loops in early-iteration networks still terminate.
 *   - The usual observers (`GetBoard`, `CurrentRound`, `CurrentPlayer`,
 *     `LastPlayer`, `LastAction`, `CanonicalBoard`, `ValidActions`,
 *     `IsOver`, `GetScore`).
 *   - Human-readable I/O (`BoardReadableString`, `ActionToString`,
 *     `ActionFromString`).
 *   - In-place mutation primitives `ApplyActionInPlace(const A&)` and
 *     `UndoLastAction()`. These are the contract used by the MCTS hot
 *     loop; both must be allocation-free.
 *
 * The non-mutating `ApplyAction(game, action)` is a free function
 * defined below and implemented once for any conforming `G`. Concrete
 * games never implement it themselves.
 *
 * `ValidActions()` must return a deterministic ordering that depends
 * only on the game state — a training tuple `(s, π, z)` written under
 * one ordering and replayed against a network trained under another
 * is corrupt.
 */
template <typename G>
concept Game = requires(G g, const G cg, typename G::action_t a,
                        typename G::player_t p) {
  // associated types
  typename G::board_t;
  typename G::action_t;
  typename G::player_t;
  typename G::error_t;

  // static contract
  { G::kHistoryLookback } -> std::convertible_to<std::size_t>;
  { G::kPolicySize } -> std::convertible_to<std::size_t>;
  { G::kMaxRounds } -> std::convertible_to<std::optional<uint32_t>>;

  // observers
  { cg.GetBoard() } -> std::convertible_to<const typename G::board_t&>;
  { cg.CurrentRound() } -> std::same_as<uint32_t>;
  { cg.CurrentPlayer() } -> std::same_as<typename G::player_t>;
  { cg.LastPlayer() } -> std::same_as<std::optional<typename G::player_t>>;
  { cg.LastAction() } -> std::same_as<std::optional<typename G::action_t>>;
  { cg.CanonicalBoard() } -> std::convertible_to<typename G::board_t>;
  { cg.ValidActions() } -> std::same_as<std::vector<typename G::action_t>>;
  { cg.IsOver() } -> std::same_as<bool>;
  { cg.GetScore(p) } -> std::same_as<float>;

  // policy-vector layout: action -> slot in [0, kPolicySize)
  { cg.PolicyIndex(a) } -> std::convertible_to<std::size_t>;

  // mutation — primary transition API; zero allocations on the hot path
  { g.ApplyActionInPlace(a) } -> std::same_as<void>;
  { g.UndoLastAction() } -> std::same_as<void>;

  // human-readable I/O
  { cg.BoardReadableString() } -> std::same_as<std::string>;
  { cg.ActionToString(a) } -> std::same_as<std::string>;
  {
    cg.ActionFromString(std::string_view{})
  } -> std::same_as<std::expected<typename G::action_t, typename G::error_t>>;
};

/**
 * @brief Non-mutating apply: copy `game`, apply `action` in place, return.
 *
 * Defined once for any conforming `Game G`. Concrete games never
 * implement this themselves — the MCTS hot loop is expected to call
 * `ApplyActionInPlace` / `UndoLastAction` on a single working copy and
 * use this free function only for cold-path snapshots.
 */
template <Game G>
[[nodiscard]] G
ApplyAction(const G& game, const typename G::action_t& action) noexcept(
    std::is_nothrow_copy_constructible_v<G>) {
  G next = game;
  next.ApplyActionInPlace(action);
  return next;
}

}  // namespace az::game::api

#endif  // ALPHA_ZERO_API_SRC_INCLUDE_ALPHA_ZERO_API_GAME_H_
