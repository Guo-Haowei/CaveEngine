#pragma once
#include "cave/core/math/Vec.h"
#include "cave/core/string/StringId.h"

#include "chess/core/Position.h"

namespace chess {

using namespace ::cave::literals;
using ::cave::StringId;
using ::cave::math::Vec3f;

constexpr StringId kTranslationId = "translation"_sid;
constexpr StringId kVisibility = "visibility"_sid;
constexpr StringId kCastShadow = "cast_shadow"_sid;

static inline Vec3f squareToVec(core::Square square) {
    const auto [file, rank] = square.fileRank();
    return Vec3f{ (float)rank, 0.0f, (float)file };
}

}  // namespace chess
