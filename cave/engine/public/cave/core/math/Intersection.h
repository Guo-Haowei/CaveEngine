#pragma once
#include "cave/core/math/Vector.h"

namespace cave::math {

class AABB;
class Plane;
class Ray;

class TestIntersection {
public:
    static bool planeRay(const Plane& plane, Ray& ray);
    static bool aabbRay(const AABB& aabb, Ray& ray);
    static bool triangleRay(const Vec3f& a, const Vec3f& b, const Vec3f& c, Ray& ray);
};

}  // namespace cave::math