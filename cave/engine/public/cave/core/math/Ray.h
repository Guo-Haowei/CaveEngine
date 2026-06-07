// =============================================================================
// File: public/cave/core/math/Ray.h
// =============================================================================
#pragma once
#include "Matrix.h"
#include "Intersection.h"

namespace cave::math {

class Ray {
public:
    Ray(const Vector3f& p_start, const Vector3f& p_end)
        : m_start(p_start), m_end(p_end), m_dist(1.0f) {}

    Ray Inverse(const Matrix4x4f& p_inverse_matrix) const;

    Vector3f Direction() const;

    bool Intersects(const AABB& p_aabb) { return TestIntersection::RayAabb(p_aabb, *this); }

    bool Intersects(const Vector3f& p_a,
                    const Vector3f& p_b,
                    const Vector3f& p_c) {
        return TestIntersection::RayTriangle(p_a, p_b, p_c, *this);
    }

    float GetDist() const { return m_dist; }
    void SetDist(float p_dist) { m_dist = p_dist; }

    static Ray Unproject(const Matrix4x4f& p_proj_view, const Vector2f& p_ndc);

private:
    const Vector3f m_start;
    const Vector3f m_end;
    float m_dist;  // hit distance

    friend class TestIntersection;
};

}  // namespace cave::math
