# AlphaZero API

[![Tests](https://github.com/shuyangsun/alpha-zero-api/actions/workflows/test.yml/badge.svg)](https://github.com/shuyangsun/alpha-zero-api/actions/workflows/test.yml)

Header-only C++ interfaces for plugging your game into an [AlphaZero](https://en.wikipedia.org/wiki/AlphaZero) engine.

## Getting Started

Want to build a game for AlphaZero? Start with [shuyangsun/alpha-zero-game](https://github.com/shuyangsun/alpha-zero-game), fire up your favorite LLM agent, type `/az-agent`, and let it cook.

## AlphaZero Engine

The engine itself is not yet open source, but this API and the game template
give contributors a clean, practical starting point for neural-network-based
reinforcement learning projects.

## Documentation

- [Public API map](doc/api-map.md) - source map for the current header-only API,
  including the `Game` concept, serializers, deserializers, augmenters, policy
  output types, ring buffer history, defaults, and example implementation.
- [Design review](doc/report.md) - historical design rationale and resolved API
  issues from the AlphaZero/AlphaGo Zero paper requirements.
- [Migration guides](doc/migration-guides/) - breaking changes and porting steps
  across released API versions.
