# AlphaZero API

[![Tests](https://github.com/shuyangsun/alpha-zero-api/actions/workflows/test.yml/badge.svg)](https://github.com/shuyangsun/alpha-zero-api/actions/workflows/test.yml)

A C++23 header-only library that defines the game interfaces needed for an [AlphaZero](https://en.wikipedia.org/wiki/AlphaZero) implementation. Implement these interfaces for your board game, and the AlphaZero engine handles the rest (MCTS, self-play, training loop).

Generate implementation template from [shuyangsun/alpha-zero-game](https://github.com/shuyangsun/alpha-zero-game).

## Requirements

- C++23 compiler (GCC 14+, Clang 18+, MSVC 19.38+)
- CMake 3.31.6+

## Installation

### CMake `ExternalProject`

The recommended way to use this library is via CMake's `ExternalProject`:

```cmake
include(ExternalProject)

ExternalProject_Add(
  AlphaZeroAPI
  PREFIX AlphaZeroAPI
  GIT_REPOSITORY https://github.com/shuyangsun/alpha-zero-api.git
  GIT_TAG main
  INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/alpha-zero-api"
  CMAKE_CACHE_ARGS
    -DDEFAULTS:BOOL=ON
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/alpha-zero-api
)
```

Then in your test/app project, find and link the package:

```cmake
find_package(AlphaZeroAPI CONFIG REQUIRED COMPONENTS api defaults)

add_executable(my_game main.cc game.cc serializer.cc)
target_link_libraries(my_game PRIVATE AlphaZeroAPI::api AlphaZeroAPI::defaults)
```

See [test/cmake_externalproject/CMakeLists.txt](test/cmake_externalproject/CMakeLists.txt) for a complete working example.

## Quick Start: Tic-Tac-Toe Example

A full Tic-Tac-Toe implementation is provided in [test/tic_tac_toe/](test/tic_tac_toe/). The typical workflow:

```text
1. Augment        — Generate equivalent board states (rotations, reflections)
2. Serialize      — Convert each board state to a float vector (NN input)
3. Inference      — Run the neural network (your responsibility)
4. Deserialize    — Convert NN output back to PolicyOutput
5. Interpret      — Merge augmented outputs into one PolicyOutput
6. Select action  — Pick the action with the highest probability
```

### Building and Running the Example

```bash
mkdir -p build && cd build
cmake -G Ninja ../test/cmake_externalproject
cmake --build .
./alpha-zero-api-tests/tic-tac-toe-main
```

## Build Options

| Option        | Default | Description                                      |
| ------------- | ------- | ------------------------------------------------ |
| `DEFAULTS`    | `OFF`   | Include default type aliases and implementations |
| `STATIC_ONLY` | `OFF`   | Install only the static library targets          |
| `SHARED_ONLY` | `OFF`   | Install only the shared library targets          |

## Components

| Component  | CMake Target             | Description                                                   |
| ---------- | ------------------------ | ------------------------------------------------------------- |
| `api`      | `AlphaZeroAPI::api`      | Core interfaces (always installed)                            |
| `defaults` | `AlphaZeroAPI::defaults` | Convenience type aliases and default implementations (opt-in) |

## Core Interfaces

All interfaces live in namespace `::az::game::api` and are templated on three types:

| Template Parameter | Meaning       | Example                                |
| ------------------ | ------------- | -------------------------------------- |
| `B`                | Board state   | `std::array<std::array<int8_t, 3>, 3>` |
| `A`                | Single action | `int16_t`, `Action2D{row, col}`        |
| `P`                | Player        | `bool` for two-player games            |

### `IGame<B, A, P>`

The main game interface. Implement this to define your game's rules.

| Method                                             | Purpose                                 |
| -------------------------------------------------- | --------------------------------------- |
| `Copy()`                                           | Deep copy the game state                |
| `GetBoard()`                                       | Current board state                     |
| `CurrentRound()`                                   | Current turn number                     |
| `CurrentPlayer()` / `LastPlayer()`                 | Player tracking                         |
| `LastAction()`                                     | Action that led to current state        |
| `CanonicalBoard()`                                 | Board from current player's perspective |
| `ValidActions()`                                   | All legal moves                         |
| `GameAfterAction(action)`                          | New game state after a move             |
| `IsOver()` / `GetScore(player)`                    | Terminal state and outcome              |
| `BoardReadableString()`                            | Human-readable board for console play   |
| `ActionFromString(str)` / `ActionToString(action)` | Console I/O helpers                     |

### `IGameSerializer<B, A, P>`

Serializes a game state into a `std::vector<float>` as neural network input.

### `IPolicyOutputSerializer<B, A, P>`

Serializes a `PolicyOutput` (value + action probabilities) into a `std::vector<float>` for training data.

### `IPolicyOutputDeserializer<B, A, P, E>`

Converts raw neural network output (`std::span<const float>`) back into a `PolicyOutput`.

### `IInferenceAugmenter<B, A, P>`

Augments a game state into multiple equivalent states before inference (e.g., rotations/reflections) to reduce model bias, then interprets the multiple outputs back into a single `PolicyOutput`.

### `ITrainingAugmenter<B, A, P>`

Augments game states and their policy outputs to produce additional training samples.

## Defaults Component

Enable with `-DDEFAULTS=ON`. Provides:

**Board types** — `Standard1DBoard<N>`, `Standard2DBoard<ROWS, COLS>`, and dynamic-size variants.

**Player/action types** — `BinaryPlayer` (`bool`), `Action1D` (`int16_t`), `Action2D` (`{row, col}`).

**Game type aliases** — e.g., `ITwoPlayer2DBoardGame<3, 3>` is `IGame<Standard2DBoard<3,3>, Action2D, BinaryPlayer>`.

**Default implementations** — `DefaultPolicyOutputSerializer` and `DefaultPolicyOutputDeserializer` for neural networks that output `[value, prob_0, prob_1, ...]` as `float`.
