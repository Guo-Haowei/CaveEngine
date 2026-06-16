#include "cave/core/math/AABB.h"
#include "cave/core/math/Frustum.h"
#include "engine/private/core/math/MatrixTransform.h"

namespace cave::math {

// https://stackoverflow.com/questions/12836967/extracting-view-frustum-planes-hartmann-gribbs-method
static Plane extractPlane(const Matrix4x4f& m,
                          int axis,
                          float sign) {
    return {
        {
            m[0][3] + sign * m[0][axis],
            m[1][3] + sign * m[1][axis],
            m[2][3] + sign * m[2][axis],

        },
        m[3][3] + sign * m[3][axis],
    };
}

Frustum::Frustum(const Matrix4x4f& pv) {
    left_ = extractPlane(pv, 0, +1.0f);
    right_ = extractPlane(pv, 0, -1.0f);

    bottom_ = extractPlane(pv, 1, +1.0f);
    top_ = extractPlane(pv, 1, -1.0f);

    near_ = extractPlane(pv, 2, +1.0f);
    far_ = extractPlane(pv, 2, -1.0f);
}

bool Frustum::intersects(const AABB& box) const {
    const Vec3f& box_min = box.Min();
    const Vec3f& box_max = box.Max();
    for (int i = 0; i < 6; ++i) {
        const Plane& plane = this->operator[](i);
        Vec3f p;
        p.x = plane.normal().x > 0.0f ? box_max.x : box_min.x;
        p.y = plane.normal().y > 0.0f ? box_max.y : box_min.y;
        p.z = plane.normal().z > 0.0f ? box_max.z : box_min.z;

        if (plane.distance(p) < 0.0f) {
            return false;
        }
    }

    return true;
}

}  // namespace cave::math
