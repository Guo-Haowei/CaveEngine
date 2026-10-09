#pragma once
#include "cave/core/math/Angle.h"
#include "cave/core/math/Vec.h"

#include "geomath.h"

// @TODO: refactor
namespace cave::math {

Mat4f LookAtRh(const Vec3f& eye, const Vec3f& center, const Vec3f& up);

Mat4f LookAtLh(const Vec3f& eye, const Vec3f& center, const Vec3f& up);

Mat4f BuildPerspectiveLH(float fovy, float aspect, float near, float far);

Mat4f BuildPerspectiveRH(float fovy, float aspect, float near, float far);

Mat4f BuildOpenGlPerspectiveRH(float fovy, float aspect, float near, float far);

Mat4f BuildOrthoRH(const float left,
                   const float right,
                   const float bottom,
                   const float top,
                   const float near,
                   const float far);

Mat4f BuildOpenGlOrthoRH(const float left,
                         const float right,
                         const float bottom,
                         const float top,
                         const float near,
                         const float far);

std::array<Mat4f, 6> BuildPointLightCubeMapViewProjectionMatrix(const Vec3f& eye, float near, float far);

std::array<Mat4f, 6> BuildOpenGlPointLightCubeMapViewProjectionMatrix(const Vec3f& eye, float near, float far);

std::array<Mat4f, 6> BuildCubeMapViewProjectionMatrix(const Vec3f& eye);

std::array<Mat4f, 6> BuildOpenGlCubeMapViewProjectionMatrix(const Vec3f& eye);

static inline Mat4f Translate(const Vec3f& vec) {
    return glm::translate(glm::vec3(vec.x, vec.y, vec.z));
}

static inline Mat4f Scale(const Vec3f& vec) {
    return glm::scale(glm::vec3(vec.x, vec.y, vec.z));
}

static inline Mat4f Rotate(const Degree& degree, const Vec3f& axis) {
    return glm::rotate(degree.radians(), glm::vec3(axis.x, axis.y, axis.z));
}

static inline Mat4f Rotate(const Radian& radians, const Vec3f& axis) {
    return glm::rotate(radians.radians(), glm::vec3(axis.x, axis.y, axis.z));
}

}  // namespace cave::math