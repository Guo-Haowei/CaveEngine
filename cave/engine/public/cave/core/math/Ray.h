// =============================================================================
// File: cave/core/math/Ray.h
// =============================================================================
#pragma once
#include "Matrix.h"
#include "Intersection.h"

namespace cave::math {

class Ray {
public:
    Ray(const Vector3f& start, const Vector3f& end)
        : origin_(start), end_(end), hit_distance_(1.0f) {}

    Ray inverse(const Matrix4x4f& inverse_matrix) const;

    Vector3f direction() const;

    bool intersects(const AABB& aabb) {
        return TestIntersection::aabbRay(aabb, *this);
    }

    bool intersects(const Plane& plane) {
        return TestIntersection::planeRay(plane, *this);
    }

    bool intersects(const Vector3f& a,
                    const Vector3f& b,
                    const Vector3f& c) {
        return TestIntersection::triangleRay(a, b, c, *this);
    }

    Vector3f hitPoint() const {
        return origin_ + hit_distance_ * direction();
    }

    float distance() const { return hit_distance_; }
    void distance(float dist) { hit_distance_ = dist; }

    static Ray unproject(const Matrix4x4f& proj_view, const Vector2f& ndc);

private:
    const Vector3f origin_;
    const Vector3f end_;
    float hit_distance_;

    friend class TestIntersection;
};

}  // namespace cave::math
