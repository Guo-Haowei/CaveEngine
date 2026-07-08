// =============================================================================
// File: cave/core/math/Matrix.h
// =============================================================================
#pragma once
#include "cave/core/typedefs.h"
#include "cave/core/math/Vec.h"

WARNING_PUSH()
WARNING_DISABLE(4201, "-Wunused-parameter")
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
WARNING_POP()

namespace cave::math {

using Mat4f = glm::mat4;

constexpr inline Vec<float, 4> operator*(const glm::mat4& lhs, const Vec<float, 4>& rhs) {
    glm::vec4 tmp(rhs.x, rhs.y, rhs.z, rhs.w);
    tmp = lhs * tmp;
    return Vec<float, 4>(tmp.x, tmp.y, tmp.z, tmp.w);
}

}  // namespace cave::math
