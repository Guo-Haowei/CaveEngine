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
    if (math::abs(denom) < epsilon()) {
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
    const Vec3f direction = ray.end_ - ray.origin_;

    Vec3f inv_d = 1.0f / direction;
    Vec3f t0s = (aabb.m_min - ray.origin_) * inv_d;
    Vec3f t1s = (aabb.m_max - ray.origin_) * inv_d;

    Vec3f tsmaller = min(t0s, t1s);
    Vec3f tbigger = max(t0s, t1s);

    const float tmin = max(-FLT_MAX, max(tsmaller.x, max(tsmaller.y, tsmaller.z)));
    const float tmax = min(FLT_MAX, min(tbigger.x, min(tbigger.y, tbigger.z)));

    // check bounding box
    if (tmin >= tmax || tmin <= 0.0f || tmin >= ray.hit_distance_) {
        return false;
    }

    ray.hit_distance_ = tmin;
    return true;
}

bool TestIntersection::triangleRay(const Vec3f& a,
                                   const Vec3f& b,
                                   const Vec3f& c,
                                   Ray& ray) {
    // P = A + u(B - A) + v(C - A) => O - A = -tD + u(B - A) + v(C - A)
    // -tD + uAB + vAC = AO
    const Vec3f direction = ray.end_ - ray.origin_;
    const Vec3f ab = b - a;
    const Vec3f ac = c - a;
    Vec3f P = cross(direction, ac);
    const float det = dot(ab, P);
    if (det < epsilon()) {
        return false;
    }

    const float inv_det = 1.0f / det;
    const Vec3f AO = ray.origin_ - a;

    const Vec3f q = cross(AO, ab);
    const float u = dot(AO, P) * inv_det;
    const float v = dot(direction, q) * inv_det;

    if (u < 0.0 || v < 0.0 || u + v > 1.0) {
        return false;
    }

    const float t = dot(ac, q) * inv_det;
    if (t < epsilon() || t >= ray.hit_distance_) {
        return false;
    }

    ray.hit_distance_ = t;
    return true;
}

}  // namespace cave::math
