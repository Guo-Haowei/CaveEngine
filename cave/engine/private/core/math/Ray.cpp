#include "cave/core/math/Ray.h"
#include "cave/core/math/Vector.h"
#include "engine/private/core/math/MatrixTransform.h"

namespace cave::math {

Vector3f Ray::direction() const {
    return normalize(end_ - origin_);
}

Ray Ray::inverse(const Matrix4x4f& inverse_matrix) const {
    Vector4f inversed_start = inverse_matrix * Vector4f(origin_, 1.0f);
    Vector4f inversed_end = inverse_matrix * Vector4f(end_, 1.0f);
    Ray inversed_ray(Vector3f(inversed_start.xyz), Vector3f(inversed_end.xyz));
    inversed_ray.hit_distance_ = hit_distance_;
    return inversed_ray;
}

Ray Ray::unproject(const Matrix4x4f& proj_view, const Vector2f& ndc) {
    const Vector4f clip_near{ ndc, 0.0f, 1.0f };
    const Vector4f clip_far{ ndc, 1.0f, 1.0f };

    const Matrix4x4f inv_pv = glm::inverse(proj_view);

    Vector4f world_near = inv_pv * clip_near;
    Vector4f world_far = inv_pv * clip_far;
    world_near /= world_near.w;
    world_far /= world_far.w;

    return { world_near.xyz, world_far.xyz };
}

}  // namespace cave::math
