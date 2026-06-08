#pragma once
#include "Vector.h"

namespace cave::math {

class AABB;
class Plane;
class Ray;

class TestIntersection {
public:
    static bool aabbAabb(const AABB& aabb1, const AABB& aabb2);
    static bool planeRay(const Plane& plane, Ray& ray);
    static bool aabbRay(const AABB& aabb, Ray& ray);
    static bool triangleRay(const Vector3f& a, const Vector3f& b, const Vector3f& c, Ray& ray);
};

}  // namespace cave::math