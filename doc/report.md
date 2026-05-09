# AlphaZero API — Design Review

A review of the C++ game-engine API in `src/include/alpha-zero-api/` against
the requirements implied by the AlphaGo Zero paper (Silver et al., 2017) and
the AlphaZero paper (Silver et al., 2018), with a secondary look at how the
current Tic-Tac-Toe example exercises the API.

The headline: the API covers the conceptual surface of a "game contract"
(state, action, transition, terminal check, canonical form, augmentation,
serialization, deserialization) and is well documented. But several pieces
that the AlphaZero paper explicitly relies on are absent or only implicit,
and a few design choices will hurt either correctness on non-toy games
(chess, Go) or performance once MCTS is doing millions of expansions.

The issues are grouped by severity. "Blocking" means it will be hard or
impossible to support a paper-faithful AlphaZero on chess/Go without
changing the API; "Significant" means the engine can work around it but at
real cost; "Polish" is naming, docs, and minor design.

---

## Planned direction (agreed)

After the initial review the following design decisions have been taken
for the next iteration of the API. They resolve B1 and B2 outright and
reshape the signatures referenced by S1 / S7 / S8. The remaining
"Blocking" items (B3, B4, B5) and the rest of the "Significant" /
"Polish" sections are still open.

1. **Drop the virtual `IGame` interface; replace it with a C++20
   `concept`** (working name `Game`). Concrete game types are plain
   value types satisfying the concept; the engine is templated on the
   concrete `G`. This unblocks stack allocation, removes
   virtual-dispatch overhead on the MCTS hot path, and lets
   `std::array<G, N>`-style storage work because `sizeof(G)` is known.

2. **All consumers (serializer, deserializer, augmenters) take
   `const G&`.** No more separate `(B, P, std::span<const A>)`
   parameter tuples — the consumer pulls whatever it needs off the
   game directly.

3. **History lives in the engine, not the game.** The `Game` concept
   exposes a `HistoryLookback()` (preferred form: a `static constexpr
std::size_t kHistoryLookback`; a runtime member-function variant is
   acceptable for games whose lookback isn't known at compile time)
   so the engine knows how deep a window to keep. API methods that
   need history take an additional `RingBufferView<const G>`
   argument. The game implementer never has to track history
   themselves; the engine is the single owner.

4. **In-place mutation is the primary transition API.**
   `ApplyActionInPlace(const A&)` and `UndoLastAction()` are the
   contract on `Game`. The non-mutating
   `G ApplyAction(const G&, const A&)` is a **free function** in the
   API library, implemented as copy-then-apply — concrete games do not
   need to provide it.

5. **Custom `RingBuffer<T, Storage>` and `RingBufferView<T>`** ship
   with the API. `RingBuffer` is parameterized on its underlying
   storage (typically `std::array<T, N>` for compile-time bounded,
   stack-friendly history; `std::vector<T>` for runtime sizing). The
   storage type is constrained by a concept so misuse fails at
   compile time. `RingBufferView<T>` is the read-only window passed
   to consumers.

The original review of B1 / B2 immediately below reflects the API
_before_ these decisions; "Suggested API surface to add" near the
bottom has been rewritten to reflect the planned shape.

---

## Blocking issues

### B1. No state history is exposed

The AlphaZero networks consume **stacks of past positions**, not just the
current board:

- AlphaGo Zero: 19 × 19 × 17 input — the last 8 own-stone planes, last 8
  opponent planes, plus turn indicator.
- AlphaZero chess: 8 × 8 × 119 input — eight time steps, each carrying
  piece planes, a repetition counter, plus castling rights, the 50-move
  rule counter, and total move count.

Today, `IGameSerializer::SerializeCurrentState(board, player, actions)`
sees only the current board snapshot. There is no way for a serializer to
reach back across earlier positions. The engine could keep an external
history vector, but the serializer signature does not accept one, so the
user's only escape hatch is to bake the entire history _into_ `B` itself
(e.g. make `B` a struct that carries the last N boards). That works, but
it is exactly the kind of contract the API should formalize, because:

1. Chess requires three-fold repetition detection — the implementation
   _must_ track history regardless. Today this is invisible to the engine.
2. Go's super-ko rule (positional or situational) requires history.
3. Once history is in `B`, every `Copy()` and `GameAfterAction()` copies
   that history too — see B2 below.

**Resolution (planned).** History is moved out of `IGame` entirely.
The game concept exposes only its required lookback depth
(`G::kHistoryLookback`); the engine maintains a `RingBuffer<G>` of
recent states and passes a `RingBufferView<const G>` to any consumer
that needs it (serializer, augmenter). Markov games declare
`kHistoryLookback == 0` and never see the history view used. Chess /
Go declare e.g. `kHistoryLookback == 8` and the engine guarantees a
window of that size is available to the serializer.

This avoids the original tension: `IGame` instances stay small and
Markov-looking (no `std::span<const B> History()` accessor), the
engine owns the history once, and the contract for "how much history
do you need?" is explicit at the type level.

### B2. `GameAfterAction()` allocates a heap object per node

```cpp
[[nodiscard]] virtual std::unique_ptr<const IGame<B,A,P,E>>
GameAfterAction(const A& action) const noexcept = 0;
```

A canonical AlphaZero training run does on the order of:

- `num_iters × games_per_iter × moves_per_game × MCTS_simulations × tree_depth`

state expansions, which is easily hundreds of millions to billions of
allocations across a training cycle. Returning `unique_ptr<const T>` from
every transition forces a heap allocation (and a virtual-dtor
non-deterministic free) on every MCTS edge. In every fast AZ engine I
know of (LeelaChessZero, KataGo), the equivalent path is either an
in-place `do_move`/`undo_move` pair or a value type with cheap copy.

Symptoms today:

- `TttGame::GameAfterAction()` builds a value, then `std::make_unique`
  copies it to the heap.
- The `Copy()` API is similarly heap-only and is needed every time MCTS
  wants to fork from the current root.

**Resolution (planned).** Take the value-semantic path, all the way:

- The virtual `IGame` interface is dropped in favor of a `Game`
  concept. Concrete game types are stack-allocated value types whose
  size is known to the compiler.
- `Game` requires in-place mutation primitives:
  `ApplyActionInPlace(const A&)` and `UndoLastAction()`. These do
  zero allocations and are what the MCTS hot loop calls.
- The non-mutating `G ApplyAction(const G&, const A&)` lives **outside**
  the game — it's a free function in the API library, defined as
  `G next = game; next.ApplyActionInPlace(action); return next;`.
  Concrete games never have to implement it.
- `Copy()` is no longer needed; copy-construction of `G` replaces it.
- The engine never holds `unique_ptr<const IGame>`; tree nodes hold
  `G` by value (or hold only an action and reconstruct the working
  state via `Apply`/`Undo` on a single working copy per simulation).

The cost paid in exchange: the engine becomes templated on the
concrete game type, which is fine — any single training run targets
one game.

### B3. `PolicyOutput` conflates value and policy and assumes a packed network output

```cpp
struct PolicyOutput {
  PolicyOutput(float value, std::vector<float>&& probabilities);
  PolicyOutput(float value, const std::vector<float>& probabilities);
  PolicyOutput(std::vector<float>&& nn_output);          // value = front()
  PolicyOutput(const std::vector<float>& nn_output);
  float value;
  std::vector<float> probabilities;
};
```

Two different concerns are tangled here:

1. **Network output layout**: real AZ networks have _separate_ heads —
   a 1-d value scalar and an N-d policy vector — typically delivered as
   two output tensors. The `PolicyOutput(vector<float>)` constructors
   bake in the assumption that they are concatenated as
   `[value, p_0, p_1, ...]`. That is one possible encoding, not a
   universal one. Worse, `nn_output.front()` is UB on an empty vector,
   and the constructors silently impose this contract on every caller.

2. **Inference vs training target**: the same `PolicyOutput` is used for
   both
   - the network's evaluation `(v, p)` produced during MCTS, and
   - the training target `(z, π)` written to the replay buffer
     (`z` is the actual game outcome, `π` is the visit-count distribution
     from the search).

   These are conceptually different — `v` is the network's prediction;
   `z` comes from `IGame::GetScore()` at game end. The deserializer doc
   correctly talks about the first; the serializer doc correctly talks
   about the second. Sharing a struct is fine, but the type name and
   docs should make it explicit, and ideally the API should distinguish
   between "what the network produced" and "what we are asking the
   network to learn."

**Suggested directions**:

- Drop the `PolicyOutput(std::vector<float>)` "first element is value"
  constructors. The deserializer is the right place to know the output
  layout — the deserializer signature can take two spans
  (`std::span<const float> value, std::span<const float> policy`) or
  remain a single span but the `PolicyOutput` itself should not encode
  the layout.
- Either rename `PolicyOutput` to something like `Evaluation` or
  `NetworkOutput`, or split it into `Evaluation` (network output) and
  `TrainingTarget` (z, π) so callers get type-level safety.
- Document explicitly:
  - `value` ∈ [−1, +1], from the **current player's** perspective;
  - `probabilities[i]` corresponds to `actions[i]` and is normalized
    over **valid actions only**.

### B4. The action ↔ policy-index mapping is an unwritten contract

The serializer produces a fixed-size policy-output vector; the
deserializer assumes its `output.size() == actions.size() + 1`. The
mapping between an `A` (e.g. `Action2D{r, c}`) and a slot in the
fixed-size vector is established **inside `SerializePolicyOutput`** and
must be inverted by the deserializer that goes back from the network's
output to a `PolicyOutput` over the current valid-action list.

Today that round-trip works only because:

- `TttSerializer::SerializePolicyOutput` writes
  `result[action.row*COLS + action.col + 1] = output.probabilities[i]`,
- and `DefaultPolicyOutputDeserializer` blindly assumes the network's
  output dimension matches `actions.size() + 1`, so it reconstructs a
  vector aligned to the _current_ `actions` list.

There are two problems:

1. **The default deserializer's contract is incompatible with the typical
   serializer.** A serializer that writes into a fixed-size N-slot policy
   (TtT does, AZ chess and Go do) produces a `1 + N`-d output (TtT: 10).
   The deserializer rejects that as a size mismatch unless
   `actions.size() - 1 == N + 1`, i.e. unless every valid-action list happens to
   be length N, only true at game start. The default is therefore unusable for
   any real game once `ValidActions()` shrinks below the full action space.
   (See`defaults/deserializer.h:46`.)
2. **There is no canonical way for the engine to know "where does action
   `a` live in the policy vector?"** The engine wants this for masking,
   training-target alignment, and symmetry handling. Today it can only
   round-trip via the user-supplied serializer/deserializer pair.

**Suggested directions**:

- Add to `IGame` (or to a new `IActionEncoder`):

  ```cpp
  virtual std::size_t PolicySize() const noexcept = 0;       // fixed N
  virtual std::size_t PolicyIndex(const A&) const noexcept = 0;
  // optional inverse for renderers / debug:
  virtual std::optional<A> ActionFromPolicyIndex(
      std::size_t idx) const noexcept = 0;
  ```

- Have the deserializer's contract become "produce a `PolicyOutput`
  whose `probabilities[i]` corresponds to `actions[i]`, given a network
  output of size `PolicySize() + value_dim`." Then the default
  deserializer is implementable for everyone.
- Optionally, replace `ValidActions() -> vector<A>` with (or alongside)
  `ValidActionMask() -> bitset/vector<bool>` of size `PolicySize()`.
  That is what the network ultimately wants, and it removes a hot-loop
  conversion in the engine.

### B5. No game-length cap surfaced

The AlphaZero paper hard-caps self-play games (e.g. 512 ply for chess) so
that pathological loops in early-iteration networks still terminate.
Today there is no `MaxRounds()` or "this game must terminate" knob on the
API. The only termination hook is `IsOver()`, and it is the
implementation's burden to enforce a cap internally. That is workable,
but the engine has no way to ask the game "what is your declared
maximum length?" (useful for replay-buffer sizing, time budgeting, and
logging).

**Suggested**: add an optional

```cpp
virtual std::optional<uint32_t> MaxRounds() const noexcept { return std::nullopt; }
```

with the documented contract that `IsOver()` must return true if
`CurrentRound() >= *MaxRounds()`.

---

## Significant issues

### S1. Augmenters should be optional, and not `unordered_map`-shaped

`IInferenceAugmenter` and `ITrainingAugmenter` are required interfaces
(the example test wires them in). Two issues:

1. Some games have no useful symmetry — chess most notably. The user is
   forced to provide a no-op identity augmenter. The API should make
   augmentation **optional**: either provide a default identity
   implementation, or make these injection points `nullable` from the
   engine's side.
2. The container choice is awkward:

   ```cpp
   std::unordered_map<uint8_t, std::tuple<B, P, std::vector<A>>>
   ```

   A `std::vector` (or `std::array<..., N>` keyed by enum) is faster,
   has stable iteration order (important for `Interpret`'s averaging,
   so values get the same weight every run), avoids hashing, and avoids
   a `uint8_t` ceiling that already feels arbitrary. The "key" is just
   "which symmetry op did I apply"; that's an enum index, not a hash key.

   Also: `Interpret(augmented_games, outputs)` requires the caller to
   carry both maps around. A cleaner shape is for `Augment` to return a
   small object with an `Interpret(span<PolicyOutput>) -> PolicyOutput`
   method on it — it already knows the augmentation set internally.

### S2. `IPolicyOutputDeserializer` claims FP16/BF16 support but the signature is `span<const float>`

```cpp
// "The output of the neural network can range from double precision to
//  half or even quarter precision. The deserializer should be able to
//  handle different neural network output data types."
[[nodiscard]] virtual std::expected<PolicyOutput, E> Deserialize(
    const B& board, const P& player, std::span<const A> actions,
    std::span<const float> output) const noexcept = 0;
```

The doc and the signature disagree. If you want to support FP16/BF16,
the signature has to be either `std::span<const std::byte>` plus a
precision flag, or templated over the element type. As written, the
contract is "callers must already up-convert to FP32," which is fine —
just say so.

### S3. `ValidActions()` ordering must be deterministic and that contract is missing

`probabilities[i]` is bound to `actions[i]`. If two calls to
`ValidActions()` on the same state can return the same set in different
orders (e.g. because the implementation iterates a hash set), then a
training tuple `(s, π, z)` written in one order and replayed against a
network trained on another order will be corrupted. Today nothing in
the docs says "ValidActions must return actions in a deterministic
order that depends only on the state." Add it.

### S4. `IGame` instances carry no equality / hashing primitives

For MCTS with a transposition table — used by KataGo, LeelaChessZero,
and most production AZ-style engines — the engine needs:

- `bool operator==(const IGame&, const IGame&)` (or a state hash)
- A Zobrist-style hash of the canonical state

These are not strictly required by the original AlphaZero paper (its
MCTS is per-rollout-fresh), but they are a near-universal optimization
for any nontrivial game. Worth at least an opt-in:

```cpp
virtual std::optional<uint64_t> StateHash() const noexcept { return std::nullopt; }
```

### S5. Value semantics for multi-player games are unspecified

The API allows `P` to be anything (e.g. `unsigned int` for an N-player
game). But:

- `GetScore(player)` returns a single `float` — implicit
  zero-/constant-sum game. Nothing says what the values should sum to.
- The `value` in `PolicyOutput` is a single scalar. For real
  multi-player AZ extensions you need a per-player value vector.

Either declare AlphaZero is two-player zero-sum only (and constrain
`P` to a binary type), or document the multi-player contract:
"`GetScore` is from `player`'s point of view; for two-player zero-sum,
`GetScore(p1) == -GetScore(p2)`; for N-player, scores must sum to a
constant independent of outcome — pick one and stick with it."

### S6. Tic-Tac-Toe inference augmenter has duplicate symmetries

This is in the example, not the API itself, but it illustrates a
foot-gun the API enables. The dihedral group D4 (square symmetries)
has order **8**, but `AugmentAll` enumerates **12** transformations.
Specifically:

- `MirrorH ∘ Rot180 == MirrorV`
- `MirrorH ∘ Rot270 == MirrorV ∘ Rot90`
- `MirrorH ∘ Rot90  == MirrorV ∘ Rot270`
- `MirrorH         == MirrorV ∘ Rot180`

So four of the twelve are duplicates of others, and `Interpret()`
averages all 12 outputs equally — giving certain symmetries double
weight. It's a real bug in the example: the value/probabilities vector
the engine ends up using is biased.

The API-design takeaway: returning a `unordered_map` keyed by a
free-form `uint8_t` makes it easy for users to over-enumerate without
the API noticing. A typed enum over the dihedral group (8 elements)
would catch this at compile time.

### S7. `SerializeCurrentState` takes `actions` but its purpose is unclear

The default TtT serializer ignores the `actions` parameter (correctly —
the network's input planes for a TtT-like game don't need an action
mask). Why is it on the interface? Two possible answers:

1. It's available for games that bake "legal moves" into input planes
   (rare but plausible).
2. It's a leftover from the `SerializePolicyOutput` shape.

Either document the intended use ("encode legal-move planes if your
network expects them"), or drop it and let implementations that need it
override and cache the actions internally. As-is, it's a parameter
that costs every implementer a thought ("do I need this?") without
guidance.

### S8. `Deserialize(board, player, actions, output)` has unused parameters in practice

Same shape as S7: `board` and `player` are passed in, but
`DefaultPolicyOutputDeserializer` only uses `actions.size()`. They're
useful only if the network's output ordering depends on board state,
which is unusual. A note in the docs ("often unused — provided so
that sparse-policy implementations can choose layouts based on
state") would prevent confusion.

---

## Polish / minor

- **`PolicyOutput` is misnamed.** It contains `value` too. Rename to
  `Evaluation` or `NetworkOutput`. Cosmetic but it confuses readers.
- **`PolicyOutput` constructor set is redundant.** Two `(value, vec&&)`
  /`(value, vec const&)` plus two `(vec)` overloads — the modern idiom
  is one constructor `(float, std::vector<float>)` taking the vector by
  value, then `std::move`. Drop half of these.
- **`PolicyOutput::PolicyOutput(const std::vector<float>&)` reads
  `nn_output.front()`** — UB on an empty vector. Either `assert`,
  return an `expected`, or remove these constructors entirely (see B3).
- **`Action1D = int16_t` is signed.** Why? If sentinels are intended,
  document them; otherwise prefer `uint16_t` for monotonicity with
  `Action2D::row/col`.
- **`ValidActions()` doc says "the returned vector can be empty if the
  current player has no valid action"** — but the TtT implementation's
  doc says the opposite ("vector should be empty if and only if the
  game is over"). The interface should pick one rule and the example
  should match it. The latter is the safer rule (engines reading
  `ValidActions().empty()` as "game over" is a common and reasonable
  shortcut).
- **`LastPlayer()` / `LastAction()` return `std::optional`**, but the
  semantic "no action yet" is identical to "game has not started." A
  single round counter check (`CurrentRound() == 0`) carries the same
  signal. The optionals are fine, but the interface could lose them
  without losing information.
- **`CanonicalBoard()` returns by value** (`B`). For large boards this
  is a copy; consider `B CanonicalBoard()` only for the default and
  add `void WriteCanonicalBoard(B& dst) const` for hot paths.
- **`BoardReadableString()` and `ActionToString()` should be `const`
  in expression but also probably `pure` — they compute deterministic
  output from state.** Fine as-is, just noting.
- **`TttGame::Create`'s only documented failure mode is
  `kNotImplemented`.** What is the actual failure case? Today
  `Create()` always succeeds. Either remove the `expected` wrapper or
  document a real failure case.
- **`TttTrainingAugmenter::Augment`** writes the _original_ `player`
  for every augmentation rather than `aug_player` from the augmented
  tuple (line 30). For TtT both are the same (mirror/rotate doesn't
  swap players), but it's brittle. The augmenter should propagate the
  augmented player.
- **`PolicyOutput` does not document whether `probabilities` is
  pre-softmax or post-softmax.** Spell it out — MCTS multiplies prior
  P(s,a), so you almost certainly want post-softmax.
- **Doc cross-refs use `api/cpp/...` paths in `augmenter.h`** — those
  don't exist; should be `alpha-zero-api/...`.

---

## What the API gets right

It's worth saying explicitly, because most of this report is criticism:

- **Const-correctness and immutability of game state** is a strong
  default. The `unique_ptr<const T>` shape makes accidental shared
  mutation of MCTS branches impossible. The cost is performance (B2)
  but the correctness story is good.
- **Templated over `B`, `A`, `P`, `E`** lets concrete games pick
  zero-overhead representations. The `defaults/game.h` aliases
  (`Standard1DBoard`, `Standard2DBoard`, etc.) are a nice ladder for
  newcomers.
- **Separation of `IGameSerializer` / `IPolicyOutputSerializer` /
  `IPolicyOutputDeserializer` / augmenters** is clean — each
  responsibility is in its own interface. Most AZ codebases tangle
  these.
- **`CanonicalBoard()` as a first-class concept** is correct and
  nontrivial — many tutorials hide this behind ad-hoc preprocessing
  in the network. Surfacing it is the right call.
- **`std::expected<A, E>` for `ActionFromString`** is the right modern
  C++ choice; the migration to a typed `E` (v0.0.5) is a real
  improvement.

---

## Suggested API surface (reflecting the planned direction)

This section sketches the new shape after the decisions above. It is
not final — names will move, and constraints in the concept will get
tightened as the engine surfaces requirements — but the shape is
representative.

### The `Game` concept (replaces virtual `IGame`)

```cpp
namespace az::game::api {

template <typename G>
concept Game = requires(G g, const G& cg,
                        typename G::action_t a, typename G::player_t p) {
  // associated types
  typename G::board_t;
  typename G::action_t;
  typename G::player_t;
  typename G::error_t;

  // history depth — compile-time per game type; Markov games use 0
  { G::kHistoryLookback } -> std::convertible_to<std::size_t>;

  // observers
  { cg.GetBoard()       } -> std::convertible_to<const typename G::board_t&>;
  { cg.CurrentRound()   } -> std::same_as<uint32_t>;
  { cg.CurrentPlayer()  } -> std::same_as<typename G::player_t>;
  { cg.LastPlayer()     } -> std::same_as<std::optional<typename G::player_t>>;
  { cg.LastAction()     } -> std::same_as<std::optional<typename G::action_t>>;
  { cg.CanonicalBoard() } -> std::convertible_to<typename G::board_t>;
  { cg.ValidActions()   } -> std::same_as<std::vector<typename G::action_t>>;
  { cg.IsOver()         } -> std::same_as<bool>;
  { cg.GetScore(p)      } -> std::same_as<float>;

  // mutation — primary transition API; zero allocations on the hot path
  { g.ApplyActionInPlace(a) } -> std::same_as<void>;
  { g.UndoLastAction()      } -> std::same_as<void>;

  // human-readable I/O
  { cg.BoardReadableString() } -> std::same_as<std::string>;
  { cg.ActionToString(a)     } -> std::same_as<std::string>;
  { cg.ActionFromString(std::string_view{}) }
      -> std::same_as<std::expected<typename G::action_t, typename G::error_t>>;

  // (B4 — still being designed; likely `kPolicySize` + `PolicyIndex(a)`
  //  hooks here so the deserializer no longer needs the runtime
  //  `actions` list.)

  // (B5 — likely `static constexpr std::optional<uint32_t> kMaxRounds`
  //  with the `IsOver()`-must-respect-it contract.)
};

// Free function — non-mutating apply, defined once for any Game.
// Concrete games do NOT implement this themselves.
template <Game G>
[[nodiscard]] G ApplyAction(const G& game,
                            const typename G::action_t& action) noexcept {
  G next = game;
  next.ApplyActionInPlace(action);
  return next;
}

}  // namespace az::game::api
```

### `RingBuffer<T, Storage>` and `RingBufferView<T>`

The engine uses these to own state history and to pass it to
serializers / augmenters. Storage is constrained at compile time so
the same buffer code works for stack-bounded and heap-resizable
backings.

```cpp
namespace az::game::api {

// Storage must be a contiguous, indexable container of T.
template <typename S>
concept RingBufferStorage = requires(S s, std::size_t i) {
  typename S::value_type;
  { s[i]     } -> std::same_as<typename S::value_type&>;
  { s.size() } -> std::convertible_to<std::size_t>;
};

// Two canonical instantiations:
//   RingBuffer<G, std::array<G, N>>   — stack, fixed capacity N
//   RingBuffer<G, std::vector<G>>     — heap, runtime capacity
template <typename T, RingBufferStorage Storage>
  requires std::same_as<typename Storage::value_type, T>
class RingBuffer {
 public:
  RingBuffer() = default;                              // for std::array
  explicit RingBuffer(std::size_t capacity);           // for std::vector

  void Push(const T& value) noexcept;
  void Push(T&& value) noexcept;

  [[nodiscard]] std::size_t Size() const noexcept;     // <= Capacity()
  [[nodiscard]] std::size_t Capacity() const noexcept;
  [[nodiscard]] bool        Empty() const noexcept;

  // Read-only window, newest-first iteration.
  [[nodiscard]] RingBufferView<T> View() const noexcept;

 private:
  Storage      storage_{};
  std::size_t  head_ = 0;
  std::size_t  size_ = 0;
};

// Cheap to copy — typically a pointer + two indices + capacity.
// Index 0 is the newest element; Size()-1 is the oldest still in
// the window.
template <typename T>
class RingBufferView {
 public:
  [[nodiscard]] std::size_t Size() const noexcept;
  [[nodiscard]] const T&    operator[](std::size_t i) const noexcept;

  // Random-access iterator support so range-based for / std::ranges
  // algorithms work directly on the view.
  class const_iterator { /* ... */ };
  [[nodiscard]] const_iterator begin() const noexcept;
  [[nodiscard]] const_iterator end()   const noexcept;
};

}  // namespace az::game::api
```

### Updated consumer signatures

Every consumer takes the game by const reference and, when relevant, a
read-only history window. The previous `(B, P, std::span<const A>)`
parameter tuples go away.

```cpp
template <Game G>
class IGameSerializer {
 public:
  virtual ~IGameSerializer() = default;
  // history.Size() == G::kHistoryLookback (engine guarantees this; for
  // games with kHistoryLookback == 0 the view is empty).
  [[nodiscard]] virtual std::vector<float> SerializeCurrentState(
      const G& game,
      RingBufferView<const G> history) const noexcept = 0;
};

template <Game G>
class IPolicyOutputSerializer {
 public:
  virtual ~IPolicyOutputSerializer() = default;
  [[nodiscard]] virtual std::vector<float> SerializePolicyOutput(
      const G& game,
      const TrainingTarget& target) const noexcept = 0;
};

template <Game G>
class IPolicyOutputDeserializer {
 public:
  virtual ~IPolicyOutputDeserializer() = default;
  [[nodiscard]] virtual std::expected<Evaluation, typename G::error_t>
  Deserialize(const G& game,
              std::span<const float> output) const noexcept = 0;
};

template <Game G>
class IInferenceAugmenter {
 public:
  virtual ~IInferenceAugmenter() = default;
  // Exact return shape TBD — likely an Augmentation handle that
  // bundles the augmented games with an Interpret() callback, so
  // callers don't have to drag two parallel containers around.
};
```

### Network output / training target split

```cpp
struct Evaluation {
  float              value;            // [-1, +1], current player's POV
  std::vector<float> probabilities;    // size == game.ValidActions().size(),
                                       // post-softmax, masked, normalized
};

struct TrainingTarget {
  float              z;                // actual game outcome, current player's POV
  std::vector<float> pi;               // MCTS visit-count distribution
};
```

Deserializers produce `Evaluation`; the policy-output serializer
accepts `TrainingTarget`. The split prevents the
"is-this-the-network-prediction-or-the-training-label?" confusion that
the single `PolicyOutput` type currently allows.

---

## TL;DR — status

Resolved (see _Planned direction_ above; concrete shape in _Suggested
API surface_):

1. ~~**Add history support (B1)**~~ — handled by `kHistoryLookback`
   on the `Game` concept plus engine-owned `RingBuffer<G>` and
   `RingBufferView<const G>` passed to consumers.
2. ~~**Add an `ApplyActionInPlace` / `UndoLastAction` pair (B2)**~~ —
   adopted as the primary transition API; the virtual `IGame`
   interface is dropped in favor of a value-semantic `Game` concept
   so games can be stack-allocated.

Still open:

1. **Formalize the action ↔ policy-index mapping (B4)** — today the
   default deserializer is broken for any non-trivial game, and the
   contract between serializer and deserializer is implicit. The
   concept sketch leaves `kPolicySize` / `PolicyIndex(a)` as a TBD
   placeholder.
2. The remaining "Significant" / "Polish" items, plus B3 (split
   `PolicyOutput` into `Evaluation` / `TrainingTarget` — partly
   addressed in the suggested API surface) and B5 (game-length cap).

Everything else is a real improvement but won't block training a
working agent.
