#pragma once

WARNING_PUSH()
WARNING_DISABLE(4201, "-Wunused-parameter")
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/vector_angle.hpp>
WARNING_POP()

#include "cave/core/math/Vec.h"
#include "cave/core/math/Matrix.h"

namespace cave::math {

using Quaternion = glm::quat;

static inline void Decompose(const Mat4f& p_matrix, Vec3f& p_scale, Vec4f& p_rotation, Vec3f& p_translation) {
    glm::vec3 scale;
    glm::vec3 translation;
    Quaternion quaternion;
    glm::vec3 _skew;
    glm::vec4 _perspective;

    glm::decompose(p_matrix, scale, quaternion, translation, _skew, _perspective);
    p_rotation.x = quaternion.x;
    p_rotation.y = quaternion.y;
    p_rotation.z = quaternion.z;
    p_rotation.w = quaternion.w;

    p_scale.set(&scale.x);
    p_translation.set(&translation.x);
}

}  // namespace cave::math
