#include "cave/core/math/Ray.h"
#include "cave/core/math/Vector.h"
#include "engine/private/core/math/MatrixTransform.h"

namespace cave::math {

Vector3f Ray::Direction() const {
    return normalize(m_end - m_start);
}

Ray Ray::Inverse(const Matrix4x4f& p_inverse_matrix) const {
    Vector4f inversed_start = p_inverse_matrix * Vector4f(m_start, 1.0f);
    Vector4f inversed_end = p_inverse_matrix * Vector4f(m_end, 1.0f);
    Ray inversed_ray(Vector3f(inversed_start.xyz), Vector3f(inversed_end.xyz));
    inversed_ray.m_dist = m_dist;
    return inversed_ray;
}

Ray Ray::Unproject(const Matrix4x4f& p_proj_view, const Vector2f& p_ndc) {
    const Vector4f clip_near{ p_ndc, 0.0f, 1.0f };
    const Vector4f clip_far{ p_ndc, 1.0f, 1.0f };

    const Matrix4x4f inv_pv = glm::inverse(p_proj_view);

    Vector4f world_near = inv_pv * clip_near;
    Vector4f world_far = inv_pv * clip_far;
    world_near /= world_near.w;
    world_far /= world_far.w;

    return { world_near.xyz, world_far.xyz };
}

}  // namespace cave::math
