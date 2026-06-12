# AlphaZero API Public API Map

Date: 2026-06-12
Status: current public header source map
Area: C++ header-only AlphaZero game API
Sources: `src/include/alpha-zero-api/game.h`,
`src/include/alpha-zero-api/serializer.h`,
`src/include/alpha-zero-api/deserializer.h`,
`src/include/alpha-zero-api/augmenter.h`,
`src/include/alpha-zero-api/policy_output.h`,
`src/include/alpha-zero-api/ring_buffer.h`,
`src/include/alpha-zero-api/defaults/`

## Summary

The AlphaZero API is a header-only C++ contract for connecting a concrete game
implementation to an AlphaZero-style engine. The central extension point is the
`az::game::api::Game<G>` concept in `game.h`; surrounding headers define state
serialization, policy-output serialization and deserialization, symmetry
augmentation, replay-buffer labels, inference evaluations, compact policy blobs,
and engine-owned history windows.

Use this document as the retrieval entry point for questions about the current
public API surface. For historical rationale and resolved design issues, see
[`doc/report.md`](report.md). For breaking upgrade steps, see the guides under
[`doc/migration-guides/`](migration-guides/).

## Public Header Map

`src/include/alpha-zero-api/game.h` defines `Game<G>`, the value-semantic game
contract. A conforming game declares `board_t`, `action_t`, `player_t`,
`error_t`, `kHistoryLookback`, `kPolicySize`, `kMaxLegalActions`, `kMaxRounds`,
state observers, human-readable I/O, `PolicyIndex(action)`,
`ApplyActionInPlace(action)`, and `UndoLastAction()`. The same header provides
the free function `ApplyAction(game, action)` for copy-then-apply snapshots.

`src/include/alpha-zero-api/policy_output.h` defines the data exchanged around
network policy and value heads. `Evaluation` is the decoded inference result:
`value` plus probabilities aligned to the actions written by
`game.ValidActionsInto(...)`. `TrainingTarget` is the self-play label: outcome
`z` plus MCTS visit distribution `pi`. `CompactPolicyTargetBlob` and
`CompactPolicyOutputBlob` support compact policy heads sized around
`G::kMaxLegalActions`.

`src/include/alpha-zero-api/serializer.h` defines `IGameSerializer<G>`,
`IPolicyOutputSerializer<G>`, and `ICompactPolicyOutputSerializer<G>`.
`IGameSerializer<G>::SerializeCurrentState(game, history)` turns the current
game plus an engine-owned `RingBufferView<G>` history window into neural-network
input. The policy serializers turn `TrainingTarget` into dense or compact
network-output training rows.

`src/include/alpha-zero-api/deserializer.h` defines
`IPolicyOutputDeserializer<G, E>` and `ICompactPolicyOutputDeserializer<G, E>`.
Deserializers decode a network forward pass into `Evaluation`, gathering or
reordering policy values so `probabilities[i]` matches the i-th legal action
written by `game.ValidActionsInto(...)`.

`src/include/alpha-zero-api/augmenter.h` defines `IInferenceAugmenter<G>` and
`ITrainingAugmenter<G>`. Inference augmentation maps one game state to symmetry
equivalent states and combines their `Evaluation`s back into the original action
ordering. Training augmentation maps `(game, TrainingTarget)` to equivalent
training examples while preserving the outcome `z`.

`src/include/alpha-zero-api/ring_buffer.h` defines `RingBuffer<T, Storage>` and
`RingBufferView<T>`, the engine-owned newest-first history window used by
serializers and other consumers that need the last `G::kHistoryLookback` states.

## Defaults And Examples

`src/include/alpha-zero-api/defaults/serializer.h` provides
`DefaultPolicyOutputSerializer<G>`, the dense canonical layout
`[z, p_0, ..., p_{kPolicySize-1}]` using `game.PolicyIndex(action)` to scatter
legal-action probabilities.

`src/include/alpha-zero-api/defaults/deserializer.h` provides
`DefaultPolicyOutputDeserializer<G>`, the dense inverse that gathers policy
values for the current legal actions and returns an `Evaluation`.

`src/include/alpha-zero-api/defaults/compact_serializer.h` and
`src/include/alpha-zero-api/defaults/compact_deserializer.h` provide the compact
policy-head equivalents for games where `kMaxLegalActions` is much smaller than
`kPolicySize`.

`src/include/alpha-zero-api/defaults/game.h` contains small board, action, and
player aliases such as `Standard1DBoard`, `Standard2DBoard`, `Action1D`,
`Action2D`, and `BinaryPlayer`.

The Tic-Tac-Toe implementation under `test/tic_tac_toe/` is the executable
example for the `Game` concept, serializers, deserializers, augmenters, and
compact policy migration patterns.

## Migration Landmarks

The current API shape is the result of several breaking changes documented in
`doc/migration-guides/`.

- `v0.0.5-to-v0.1.0.md` replaced the virtual `IGame` interface with
  `Game<G>`, split `PolicyOutput` into `Evaluation` and `TrainingTarget`, added
  `kHistoryLookback`, `kPolicySize`, `kMaxRounds`, and formalized
  `PolicyIndex(action)`.
- `v0.1.0-to-v0.2.0.md` added `G::kMaxLegalActions` and compact policy-output
  interfaces.
- `v0.2.0-to-v0.2.1.md` replaced heap-allocating `ValidActions()` with
  caller-owned `ValidActionsInto(std::array<action_t, kMaxLegalActions>&)`.
