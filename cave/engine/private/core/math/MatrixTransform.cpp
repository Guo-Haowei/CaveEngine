#include "MatrixTransform.h"

// @TODO: refactor
namespace cave::math {

Mat4f LookAtRh(const Vec3f& eye, const Vec3f& center, const Vec3f& up) {
#define C(v) glm::vec3(v.x, v.y, v.z)
    return glm::lookAtRH(C(eye), C(center), C(up));
#undef C
}

Mat4f LookAtLh(const Vec3f& eye, const Vec3f& center, const Vec3f& up) {
#define VEC(v) glm::vec3(v.x, v.y, v.z)
    return glm::lookAtLH(VEC(eye), VEC(center), VEC(up));
#undef VEC
}

Mat4f BuildPerspectiveLH(float fovy, float aspect, float z_near, float z_far) {
    const float tan_half_fovy = glm::tan(0.5f * fovy);
    Mat4f result(0.0f);
    result[0][0] = 1.0f / (aspect * tan_half_fovy);
    result[1][1] = 1.0f / tan_half_fovy;
    result[2][2] = z_far / (z_far - z_near);
    result[2][3] = 1.0f;
    result[3][2] = -(z_far * z_near) / (z_far - z_near);
    return result;
}

Mat4f BuildPerspectiveRH(float fovy, float aspect, float z_near, float z_far) {
    const float tan_half_fovy = glm::tan(0.5f * fovy);
    Mat4f result(0.0f);
    result[0][0] = 1.0f / (aspect * tan_half_fovy);
    result[1][1] = 1.0f / tan_half_fovy;
    result[2][2] = -z_far / (z_far - z_near);
    result[2][3] = -1.0f;
    result[3][2] = -(z_far * z_near) / (z_far - z_near);

    return result;
}

Mat4f BuildOpenGlPerspectiveRH(float fovy, float aspect, float z_near, float z_far) {
    const float tan_half_fovy = glm::tan(0.5f * fovy);
    Mat4f result(0.0f);
    result[0][0] = 1.0f / (aspect * tan_half_fovy);
    result[1][1] = 1.0f / tan_half_fovy;
    result[2][2] = -(z_far + z_near) / (z_far - z_near);
    result[2][3] = -1.0f;
    result[3][2] = -(2.0f * z_far * z_near) / (z_far - z_near);
    return result;
}

Mat4f BuildOrthoRH(const float left,
                   const float right,
                   const float bottom,
                   const float top,
                   const float z_near,
                   const float z_far) {

    const float reciprocal_width = 1.0f / (right - left);
    const float reciprocal_height = 1.0f / (top - bottom);
    const float reciprocal_depth = 1.0f / (z_far - z_near);

    Mat4f result(1.0f);
    result[0][0] = 2.0f * reciprocal_width;
    result[1][1] = 2.0f * reciprocal_height;
    result[2][2] = -1.0f * reciprocal_depth;
    result[3][0] = -(right + left) * reciprocal_width;
    result[3][1] = -(top + bottom) * reciprocal_height;
    result[3][2] = -z_near * reciprocal_depth;
    return result;
}

Mat4f BuildOpenGlOrthoRH(const float left,
                         const float right,
                         const float bottom,
                         const float top,
                         const float z_near,
                         const float z_far) {

    const float reciprocal_width = 1.0f / (right - left);
    const float reciprocal_height = 1.0f / (top - bottom);
    const float reciprocal_depth = 1.0f / (z_far - z_near);

    Mat4f result(1.0f);
    result[0][0] = 2.0f * reciprocal_width;
    result[1][1] = 2.0f * reciprocal_height;
    result[2][2] = -2.0f * reciprocal_depth;
    result[3][0] = -(right + left) * reciprocal_width;
    result[3][1] = -(top + bottom) * reciprocal_height;
    result[3][2] = -(z_far + z_near) * reciprocal_depth;
    return result;
}

std::array<Mat4f, 6> BuildPointLightCubeMapViewProjectionMatrix(const Vec3f& eye, float z_near, float z_far) {
    auto P = BuildPerspectiveLH(glm::radians(90.0f), 1.0f, z_near, z_far);

    std::array<Mat4f, 6> matrices = {
        P * LookAtLh(eye, eye + Vec3f(+1, +0, +0), Vec3f(0, +1, +0)),
        P * LookAtLh(eye, eye + Vec3f(-1, +0, +0), Vec3f(0, +1, +0)),
        P * LookAtLh(eye, eye + Vec3f(+0, +1, +0), Vec3f(0, +0, -1)),
        P * LookAtLh(eye, eye + Vec3f(+0, -1, +0), Vec3f(0, +0, +1)),
        P * LookAtLh(eye, eye + Vec3f(+0, +0, +1), Vec3f(0, +1, +0)),
        P * LookAtLh(eye, eye + Vec3f(+0, +0, -1), Vec3f(0, +1, +0)),
    };

    return matrices;
}

std::array<Mat4f, 6> BuildOpenGlPointLightCubeMapViewProjectionMatrix(const Vec3f& eye, float z_near, float z_far) {
    auto P = BuildOpenGlPerspectiveRH(glm::radians(90.0f), 1.0f, z_near, z_far);

    std::array<Mat4f, 6> matrices = {
        P * LookAtRh(eye, eye + Vec3f(+1, +0, +0), Vec3f(0, -1, +0)),
        P * LookAtRh(eye, eye + Vec3f(-1, +0, +0), Vec3f(0, -1, +0)),
        P * LookAtRh(eye, eye + Vec3f(+0, +1, +0), Vec3f(0, +0, +1)),
        P * LookAtRh(eye, eye + Vec3f(+0, -1, +0), Vec3f(0, +0, -1)),
        P * LookAtRh(eye, eye + Vec3f(+0, +0, +1), Vec3f(0, -1, +0)),
        P * LookAtRh(eye, eye + Vec3f(+0, +0, -1), Vec3f(0, -1, +0)),
    };

    return matrices;
}

std::array<Mat4f, 6> BuildCubeMapViewProjectionMatrix(const Vec3f& eye) {
    auto P = BuildPerspectiveRH(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

    std::array<Mat4f, 6> matrices = {
        P * LookAtRh(eye, eye + Vec3f(+1, +0, +0), Vec3f(0, -1, +0)),
        P * LookAtRh(eye, eye + Vec3f(-1, +0, +0), Vec3f(0, -1, +0)),
        P * LookAtRh(eye, eye + Vec3f(+0, -1, +0), Vec3f(0, +0, -1)),
        P * LookAtRh(eye, eye + Vec3f(+0, +1, +0), Vec3f(0, +0, +1)),
        P * LookAtRh(eye, eye + Vec3f(+0, +0, +1), Vec3f(0, -1, +0)),
        P * LookAtRh(eye, eye + Vec3f(+0, +0, -1), Vec3f(0, -1, +0)),
    };

    return matrices;
}

std::array<Mat4f, 6> BuildOpenGlCubeMapViewProjectionMatrix(const Vec3f& eye) {
    auto P = BuildOpenGlPerspectiveRH(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

    std::array<Mat4f, 6> matrices = {
        P * LookAtRh(eye, eye + Vec3f(+1, +0, +0), Vec3f(0, -1, +0)),
        P * LookAtRh(eye, eye + Vec3f(-1, +0, +0), Vec3f(0, -1, +0)),
        P * LookAtRh(eye, eye + Vec3f(+0, +1, +0), Vec3f(0, +0, +1)),
        P * LookAtRh(eye, eye + Vec3f(+0, -1, +0), Vec3f(0, +0, -1)),
        P * LookAtRh(eye, eye + Vec3f(+0, +0, +1), Vec3f(0, -1, +0)),
        P * LookAtRh(eye, eye + Vec3f(+0, +0, -1), Vec3f(0, -1, +0)),
    };

    return matrices;
}

#if 0
inline std::array<Mat4f, 6> BuildCubeMapViewMatrices(const Vec3f& eye) {
    std::array<Mat4f, 6> matrices;

#if 0
    matrices[0] = glm::lookAtLH(eye, eye + Vec3f(+1, +0, +0), Vec3f(0, +1, +0));
    matrices[1] = glm::lookAtLH(eye, eye + Vec3f(-1, +0, +0), Vec3f(0, +1, +0));
    matrices[2] = glm::lookAtLH(eye, eye + Vec3f(+0, +1, +0), Vec3f(0, +0, -1));
    matrices[3] = glm::lookAtLH(eye, eye + Vec3f(+0, -1, +0), Vec3f(0, +0, +1));
    matrices[4] = glm::lookAtLH(eye, eye + Vec3f(+0, +0, +1), Vec3f(0, +1, +0));
    matrices[5] = glm::lookAtLH(eye, eye + Vec3f(+0, +0, -1), Vec3f(0, +1, +0));
#else
    matrices[0] = glm::lookAt(eye, eye + Vec3f(+1, +0, +0), Vec3f(0, -1, +0));
    matrices[1] = glm::lookAt(eye, eye + Vec3f(-1, +0, +0), Vec3f(0, -1, +0));
    matrices[2] = glm::lookAt(eye, eye + Vec3f(+0, -1, +0), Vec3f(0, +0, -1));
    matrices[3] = glm::lookAt(eye, eye + Vec3f(+0, +1, +0), Vec3f(0, +0, +1));
    matrices[4] = glm::lookAt(eye, eye + Vec3f(+0, +0, +1), Vec3f(0, -1, +0));
    matrices[5] = glm::lookAt(eye, eye + Vec3f(+0, +0, -1), Vec3f(0, -1, +0));
#endif
    return matrices;
}

inline std::array<Mat4f, 6> BuildOpenGlCubeMapViewMatrices(const Vec3f& eye) {
    std::array<Mat4f, 6> matrices;
    matrices[0] = glm::lookAtRH(eye, eye + Vec3f(+1, +0, +0), Vec3f(0, -1, +0));
    matrices[1] = glm::lookAtRH(eye, eye + Vec3f(-1, +0, +0), Vec3f(0, -1, +0));
    matrices[2] = glm::lookAtRH(eye, eye + Vec3f(+0, +1, +0), Vec3f(0, +0, +1));
    matrices[3] = glm::lookAtRH(eye, eye + Vec3f(+0, -1, +0), Vec3f(0, +0, -1));
    matrices[4] = glm::lookAtRH(eye, eye + Vec3f(+0, +0, +1), Vec3f(0, -1, +0));
    matrices[5] = glm::lookAtRH(eye, eye + Vec3f(+0, +0, -1), Vec3f(0, -1, +0));
    return matrices;
}
#endif

}  // namespace cave::math