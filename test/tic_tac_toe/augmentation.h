#ifndef ALPHA_ZERO_API_TEST_TIC_TAC_TOE_AUGMENTATION_H_
#define ALPHA_ZERO_API_TEST_TIC_TAC_TOE_AUGMENTATION_H_

#include <cstdint>
#include <span>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "game.h"

namespace az::game::api::test::internal {

enum class Augmentation : uint8_t {
  kOriginal = 0,
  kRotate90,
  kRotate180,
  kRotate270,
  kMirrorHorizontal,
  kMirrorHorizontalRotate90,
  kMirrorHorizontalRotate180,
  kMirrorHorizontalRotate270,
  kMirrorVertical,
  kMirrorVerticalRotate90,
  kMirrorVerticalRotate180,
  kMirrorVerticalRotate270
};

TttAction MirrorHorizontal(const TttAction& action);
TttAction MirrorVertical(const TttAction& action);
TttAction RotateClockwise(const TttAction& action);
TttAction RotateCounterclockwise(const TttAction& action, size_t times);

std::tuple<TttBoard, TttPlayer, std::vector<TttAction>> MirrorHorizontal(
    const TttBoard& board, const TttPlayer& player,
    std::span<const TttAction> actions);

std::tuple<TttBoard, TttPlayer, std::vector<TttAction>> MirrorVertical(
    const TttBoard& board, const TttPlayer& player,
    std::span<const TttAction> actions);

std::tuple<TttBoard, TttPlayer, std::vector<TttAction>> RotateClockwise(
    const TttBoard& board, const TttPlayer& player,
    std::span<const TttAction> actions);

// Applies all 12 augmentations (original, 3 rotations, mirror-H,
// mirror-H + 3 rotations, mirror-V, mirror-V + 3 rotations).
// Keys are Augmentation enum values cast to uint8_t.
std::unordered_map<uint8_t,
                   std::tuple<TttBoard, TttPlayer, std::vector<TttAction>>>
AugmentAll(const TttBoard& board, const TttPlayer& player,
           std::span<const TttAction> actions);

}  // namespace az::game::api::test::internal

#endif  // ALPHA_ZERO_API_TEST_TIC_TAC_TOE_AUGMENTATION_H_
