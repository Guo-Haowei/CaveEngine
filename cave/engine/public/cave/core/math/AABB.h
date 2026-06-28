#pragma once
#include "Box.h"
#include "Intersection.h"
#include "Matrix.h"

namespace cave::math {

class AABB : public Box3 {
public:
    using Box3::Box;

    void applyMatrix(const Mat4f& mat);

    bool intersects(Ray& ray) const { return TestIntersection::aabbRay(*this, ray); }

    static AABB fromCenterSize(const Vec3f& center, const Vec3f& size);
};

}  // namespace cave::math
