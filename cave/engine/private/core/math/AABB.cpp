#include "cave/core/math/AABB.h"

#include "engine/private/core/math/MatrixTransform.h"

namespace cave::math {

#if 0
 *        E__________________ H
 *       /|                 /|
 *      / |                / |
 *     /  |               /  |
 *   A/___|______________/D  |
 *    |   |              |   |
 *    |   |              |   |
 *    |   |              |   |
 *    |  F|______________|___|G
 *    |  /               |  /
 *    | /                | /
 *   B|/_________________|C
 *
 * A, B, B, C, C, D, D, A, E, F, F, G, G, H, H, E, A, E, B, F, D, H, C, G
#endif
#if 0
Vector3f AABB::Corner(int p_index) const {
    // clang-format off
    switch (p_index)
    {
        case 0: return Vector3f(m_min.x, m_max.y, m_max.z); // A
        case 1: return Vector3f(m_min.x, m_min.y, m_max.z); // B
        case 2: return Vector3f(m_max.x, m_min.y, m_max.z); // C
        case 3: return Vector3f(m_max.x, m_max.y, m_max.z); // D
        case 4: return Vector3f(m_min.x, m_max.y, m_min.z); // E
        case 5: return Vector3f(m_min.x, m_min.y, m_min.z); // F
        case 6: return Vector3f(m_max.x, m_min.y, m_min.z); // G
        case 7: return Vector3f(m_max.x, m_max.y, m_min.z); // H
    }
    // clang-format on
    DEV_ASSERT(0);
    return Vector3f(0);
}
#endif

void AABB::applyMatrix(const Mat4f& p_mat4) {
    const Vec4f points[] = { Vec4f(min_.x, min_.y, min_.z, 1.0f), Vec4f(min_.x, min_.y, max_.z, 1.0f),
                             Vec4f(min_.x, max_.y, min_.z, 1.0f), Vec4f(min_.x, max_.y, max_.z, 1.0f),
                             Vec4f(max_.x, min_.y, min_.z, 1.0f), Vec4f(max_.x, min_.y, max_.z, 1.0f),
                             Vec4f(max_.x, max_.y, min_.z, 1.0f), Vec4f(max_.x, max_.y, max_.z, 1.0f) };
    static_assert(std::size(points) == 8);

    AABB new_box;
    for (size_t i = 0; i < std::size(points); ++i) {
        auto point = p_mat4 * points[i];
        new_box.expandToInclude(Vec3f(point.x, point.y, point.z));
    }

    min_ = new_box.min_;
    max_ = new_box.max_;
}

AABB AABB::fromCenterSize(const Vec3f& p_center, const Vec3f& p_size) {
    AABB box;
    Vec3f center;
    Vec3f half_size;
    center.set(&p_center.x);
    half_size.set(&p_size.x);
    half_size *= 0.5f;

    box.min_ = center - half_size;
    box.max_ = center + half_size;
    return box;
}

}  // namespace cave::math
