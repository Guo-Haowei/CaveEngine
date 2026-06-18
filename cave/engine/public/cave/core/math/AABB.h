#pragma once
#include "Box.h"
#include "Intersection.h"
#include "Matrix.h"

namespace cave::math {

class AABB : public Box3 {
public:
    using Box3::Box;

    void ApplyMatrix(const Mat4f& p_mat);

    bool Intersects(const AABB& p_aabb) const { return TestIntersection::aabbAabb(*this, p_aabb); }
    bool Intersects(Ray& p_ray) const { return TestIntersection::aabbRay(*this, p_ray); }

    static AABB FromCenterSize(const Vec3f& p_center, const Vec3f& p_size);

    friend class TestIntersection;
};

}  // namespace cave::math
