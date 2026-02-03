#include "cave/runtime/scene/CameraComponent.h"

#include "engine/private/core/math/MatrixTransform.h"

namespace cave {

using math::Matrix4x4f;
using math::Vector3f;
using math::Vector4f;

Matrix4x4f CameraComponent::CalcProjection() const {
    if (m_projection == ProjectionType::Orthographic) {
        const float half_height = m_ortho_height * 0.5f;
        const float half_width = half_height * GetAspect();
        return math::BuildOrthoRH(-half_width,
                                  half_width,
                                  -half_height,
                                  half_height,
                                  m_near,
                                  m_far);
    }
    return math::BuildPerspectiveRH(glm::radians<float>(m_fovy), GetAspect(), m_near, m_far);
}

Matrix4x4f CameraComponent::CalcProjectionGL() const {
    if (m_projection == ProjectionType::Orthographic) {
        const float half_height = m_ortho_height * 0.5f;
        const float half_width = half_height * GetAspect();
        return math::BuildOpenGlOrthoRH(-half_width,
                                        half_width,
                                        -half_height,
                                        half_height,
                                        m_near,
                                        m_far);
    }
    return math::BuildOpenGlPerspectiveRH(glm::radians<float>(m_fovy), GetAspect(), m_near, m_far);
}

bool CameraComponent::Update(const math::Matrix4x4f& p_transform) {
    // @NOTE: the logic is wrong here,
    // if the transform has changed, the cache must update too
    if (IsDirty()) {
        SetDirty(false);

        m_front = (p_transform * -Vector4f::UnitZ).xyz;
        m_right = (p_transform * Vector4f::UnitX).xyz;
        m_up = (p_transform * Vector4f::UnitY).xyz;
        m_position = (p_transform * Vector4f::UnitW).xyz;

        m_view_matrix = LookAtRh(m_position, m_position + m_front, Vector3f::UnitY);

        // use gl matrix for frustum culling
        m_projection_matrix = CalcProjectionGL();
        m_projection_view_matrix = m_projection_matrix * m_view_matrix;
        return true;
    }

    return false;
}

// @TODO: reflection?
void CameraComponent::SetOrthoHeight(float p_height) {
    if (p_height != m_ortho_height) {
        m_ortho_height = p_height;
        SetDirty();
    }
}

}  // namespace cave
