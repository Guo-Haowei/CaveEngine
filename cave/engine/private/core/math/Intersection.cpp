#include "cave/core/math/AABB.h"
#include "cave/core/math/Intersection.h"
#include "cave/core/math/Plane.h"
#include "cave/core/math/Ray.h"

namespace cave::math {

bool TestIntersection::aabbAabb(const AABB& aabb1, const AABB& aabb2) {
    AABB tmp{ aabb1 };
    tmp.IntersectBox(aabb2);
    bool result = true;
    result = result && tmp.m_min.x < tmp.m_max.x;
    result = result && tmp.m_min.y < tmp.m_max.y;
    result = result && tmp.m_min.z < tmp.m_max.z;
    return result;
}

bool TestIntersection::planeRay(const Plane& plane, Ray& ray) {
    const float denom = math::dot(plane.normal(), ray.direction());
    if (math::abs(denom) < 1e-6f) {
        return false;  // parallel
    }

    const float t = -plane.distance(ray.origin_) / denom;
    if (t < 0.0f) {
        return false;  // behind ray
    }

    ray.distance(t);
    return true;
}

bool TestIntersection::aabbRay(const AABB& aabb, Ray& ray) {
    const Vector3f direction = ray.end_ - ray.origin_;

    Vector3f inv_d = 1.0f / direction;
    Vector3f t0s = (aabb.m_min - ray.origin_) * inv_d;
    Vector3f t1s = (aabb.m_max - ray.origin_) * inv_d;

    Vector3f tsmaller = min(t0s, t1s);
    Vector3f tbigger = max(t0s, t1s);

    const float tmin = max(-FLT_MAX, max(tsmaller.x, max(tsmaller.y, tsmaller.z)));
    const float tmax = min(FLT_MAX, min(tbigger.x, min(tbigger.y, tbigger.z)));

    // check bounding box
    if (tmin >= tmax || tmin <= 0.0f || tmin >= ray.hit_distance_) {
        return false;
    }

    ray.hit_distance_ = tmin;
    return true;
}

bool TestIntersection::triangleRay(const Vector3f& a,
                                   const Vector3f& b,
                                   const Vector3f& c,
                                   Ray& ray) {
    // P = A + u(B - A) + v(C - A) => O - A = -tD + u(B - A) + v(C - A)
    // -tD + uAB + vAC = AO
    const Vector3f direction = ray.end_ - ray.origin_;
    const Vector3f ab = b - a;
    const Vector3f ac = c - a;
    Vector3f P = cross(direction, ac);
    const float det = dot(ab, P);
    if (det < Epsilon()) {
        return false;
    }

    const float inv_det = 1.0f / det;
    const Vector3f AO = ray.origin_ - a;

    const Vector3f q = cross(AO, ab);
    const float u = dot(AO, P) * inv_det;
    const float v = dot(direction, q) * inv_det;

    if (u < 0.0 || v < 0.0 || u + v > 1.0) {
        return false;
    }

    const float t = dot(ac, q) * inv_det;
    if (t < Epsilon() || t >= ray.hit_distance_) {
        return false;
    }

    ray.hit_distance_ = t;
    return true;
}

}  // namespace cave::math
