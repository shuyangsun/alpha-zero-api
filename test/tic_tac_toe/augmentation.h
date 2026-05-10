#ifndef ALPHA_ZERO_API_TEST_TIC_TAC_TOE_AUGMENTATION_H_
#define ALPHA_ZERO_API_TEST_TIC_TAC_TOE_AUGMENTATION_H_

#include <cstddef>
#include <cstdint>
#include <vector>

#include "tic_tac_toe/game.h"

namespace az::game::api::test::internal {

/**
 * @brief Symmetry tags used by augmenters. Order matches the order of
 * the games returned by `AugmentAll`.
 *
 * Note: this enumerates all 12 transformations historically applied
 * by the Tic-Tac-Toe example. The dihedral group D4 has order 8, so
 * the last four entries (kMirrorVertical*) are duplicates of earlier
 * entries (`MirrorVertical = MirrorHorizontal * Rot180`, etc.). The
 * duplicates are kept here to preserve historical behavior;
 * `Interpret` weights all 12 outputs equally.
 */
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
  kMirrorVerticalRotate270,
};

constexpr std::size_t kNumAugmentations = 12;

[[nodiscard]] TttAction MirrorHorizontal(const TttAction& action) noexcept;
[[nodiscard]] TttAction MirrorVertical(const TttAction& action) noexcept;
[[nodiscard]] TttAction RotateClockwise(const TttAction& action) noexcept;
[[nodiscard]] TttAction RotateCounterclockwise(const TttAction& action,
                                               std::size_t times) noexcept;

[[nodiscard]] TttGame MirrorHorizontal(const TttGame& game) noexcept;
[[nodiscard]] TttGame MirrorVertical(const TttGame& game) noexcept;
[[nodiscard]] TttGame RotateClockwise(const TttGame& game) noexcept;

/**
 * @brief Produce all 12 augmented `TttGame` snapshots in the order
 * given by `Augmentation`. `result[i]` corresponds to `Augmentation(i)`.
 */
[[nodiscard]] std::vector<TttGame> AugmentAll(const TttGame& game) noexcept;

/**
 * @brief Map an action expressed in an augmented coordinate frame
 * back to the original (unaugmented) coordinate frame.
 */
[[nodiscard]] TttAction InverseTransformAction(
    const TttAction& augmented_action, Augmentation augmentation) noexcept;

}  // namespace az::game::api::test::internal

#endif  // ALPHA_ZERO_API_TEST_TIC_TAC_TOE_AUGMENTATION_H_
