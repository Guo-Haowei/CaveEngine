// =============================================================================
// File: cave/core/math/Ray.h
// =============================================================================
#pragma once
#include "Matrix.h"
#include "Intersection.h"

namespace cave::math {

class Ray {
public:
    Ray(const Vec3f& start, const Vec3f& end)
        : origin_(start), end_(end), hit_distance_(1.0f) {}

    Ray inverse(const Matrix4x4f& inverse_matrix) const;

    Vec3f direction() const;

    bool intersects(const AABB& aabb) {
        return TestIntersection::aabbRay(aabb, *this);
    }

    bool intersects(const Plane& plane) {
        return TestIntersection::planeRay(plane, *this);
    }

    bool intersects(const Vec3f& a,
                    const Vec3f& b,
                    const Vec3f& c) {
        return TestIntersection::triangleRay(a, b, c, *this);
    }

    Vec3f hitPoint() const {
        return origin_ + hit_distance_ * direction();
    }

    float distance() const { return hit_distance_; }
    void distance(float dist) { hit_distance_ = dist; }

    static Ray unproject(const Matrix4x4f& proj_view, const Vec2f& ndc);

private:
    const Vec3f origin_;
    const Vec3f end_;
    float hit_distance_;

    friend class TestIntersection;
};

}  // namespace cave::math
