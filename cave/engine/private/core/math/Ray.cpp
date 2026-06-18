#include "cave/core/math/Ray.h"
#include "cave/core/math/Vector.h"
#include "engine/private/core/math/MatrixTransform.h"

namespace cave::math {

Vec3f Ray::direction() const {
    return normalize(end_ - origin_);
}

Ray Ray::inverse(const Mat4f& inverse_matrix) const {
    Vec4f inversed_start = inverse_matrix * Vec4f(origin_, 1.0f);
    Vec4f inversed_end = inverse_matrix * Vec4f(end_, 1.0f);
    Ray inversed_ray(Vec3f(inversed_start.xyz), Vec3f(inversed_end.xyz));
    inversed_ray.hit_distance_ = hit_distance_;
    return inversed_ray;
}

Ray Ray::unproject(const Mat4f& proj_view, const Vec2f& ndc) {
    const Vec4f clip_near{ ndc, 0.0f, 1.0f };
    const Vec4f clip_far{ ndc, 1.0f, 1.0f };

    const Mat4f inv_pv = glm::inverse(proj_view);

    Vec4f world_near = inv_pv * clip_near;
    Vec4f world_far = inv_pv * clip_far;
    world_near /= world_near.w;
    world_far /= world_far.w;

    return { world_near.xyz, world_far.xyz };
}

}  // namespace cave::math
