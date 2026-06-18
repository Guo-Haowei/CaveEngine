#pragma once
#include "Matrix.h"
#include "Plane.h"

namespace cave::math {

class AABB;

class Frustum {
public:
    Frustum() = default;
    Frustum(const Mat4f& pv);

    Plane& operator[](int index) { return reinterpret_cast<Plane*>(this)[index]; }

    const Plane& operator[](int index) const { return reinterpret_cast<const Plane*>(this)[index]; }

    bool intersects(const AABB& box) const;

private:
    Plane left_;
    Plane right_;
    Plane top_;
    Plane bottom_;
    Plane near_;
    Plane far_;
};

}  // namespace cave::math
