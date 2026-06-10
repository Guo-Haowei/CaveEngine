#pragma once
#include "cave/core/math/Vector.h"
#include "cave/core/string/StringId.h"

#include "chess/core/Position.h"

namespace chess {

using namespace ::cave::literals;
using ::cave::StringId;
using ::cave::math::Vector3f;

constexpr StringId kTranslationId = "translation"_sid;
constexpr StringId kVisibility = "visibility"_sid;
constexpr StringId kCastShadow = "cast_shadow"_sid;

static inline Vector3f squareToVec(core::Square square) {
    const auto [file, rank] = square.fileRank();
    return Vector3f{ (float)rank, 0.0f, (float)file };
}

}  // namespace chess
