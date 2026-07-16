#pragma once
#include "cave/core/math/Vec.h"
#include "cave/core/string/StringId.h"

#include "chess/core/Position.h"

namespace chess {

using ::cave::StringId;
using ::cave::math::Vec3f;

constexpr StringId kTranslationId = CAVE_SID("translation");
constexpr StringId kScaleId = CAVE_SID("scale");
constexpr StringId kRotationId = CAVE_SID("rotation");
constexpr StringId kVisibility = CAVE_SID("visibility");
constexpr StringId kCastShadow = CAVE_SID("cast_shadow");
constexpr StringId kTransparency = CAVE_SID("transparency");

static inline Vec3f squareToVec(core::Square square) {
    const auto [file, rank] = square.fileRank();
    return Vec3f{ (float)rank, 0.0f, (float)file };
}

}  // namespace chess
