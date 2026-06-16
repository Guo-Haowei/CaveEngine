#pragma once
#include "cave/core/math/Angle.h"
#include "cave/core/math/Vector.h"

#include "geomath.h"

// @TODO: refactor
namespace cave::math {

Matrix4x4f LookAtRh(const Vec3f& p_eye, const Vec3f& p_center, const Vec3f& p_up);

Matrix4x4f LookAtLh(const Vec3f& p_eye, const Vec3f& p_center, const Vec3f& p_up);

Matrix4x4f BuildPerspectiveLH(float p_fovy, float p_aspect, float p_near, float p_far);

Matrix4x4f BuildPerspectiveRH(float p_fovy, float p_aspect, float p_near, float p_far);

Matrix4x4f BuildOpenGlPerspectiveRH(float p_fovy, float p_aspect, float p_near, float p_far);

Matrix4x4f BuildOrthoRH(const float p_left,
                        const float p_right,
                        const float p_bottom,
                        const float p_top,
                        const float p_near,
                        const float p_far);

Matrix4x4f BuildOpenGlOrthoRH(const float p_left,
                              const float p_right,
                              const float p_bottom,
                              const float p_top,
                              const float p_near,
                              const float p_far);

std::array<Matrix4x4f, 6> BuildPointLightCubeMapViewProjectionMatrix(const Vec3f& p_eye, float p_near, float p_far);

std::array<Matrix4x4f, 6> BuildOpenGlPointLightCubeMapViewProjectionMatrix(const Vec3f& p_eye, float p_near, float p_far);

std::array<Matrix4x4f, 6> BuildCubeMapViewProjectionMatrix(const Vec3f& p_eye);

std::array<Matrix4x4f, 6> BuildOpenGlCubeMapViewProjectionMatrix(const Vec3f& p_eye);

static inline Matrix4x4f Translate(const Vec3f& p_vec) {
    return glm::translate(glm::vec3(p_vec.x, p_vec.y, p_vec.z));
}

static inline Matrix4x4f Scale(const Vec3f& p_vec) {
    return glm::scale(glm::vec3(p_vec.x, p_vec.y, p_vec.z));
}

static inline Matrix4x4f Rotate(const Degree& p_degree, const Vec3f& p_axis) {
    return glm::rotate(p_degree.radians(), glm::vec3(p_axis.x, p_axis.y, p_axis.z));
}

static inline Matrix4x4f Rotate(const Radian& p_radians, const Vec3f& p_axis) {
    return glm::rotate(p_radians.radians(), glm::vec3(p_axis.x, p_axis.y, p_axis.z));
}

}  // namespace cave::math