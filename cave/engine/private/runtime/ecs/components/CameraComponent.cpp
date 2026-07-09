#include "cave/runtime/ecs/components/CameraComponent.h"

#include "engine/private/core/math/MatrixTransform.h"

namespace cave {

using math::Mat4f;
using math::Vec3f;
using math::Vec4f;

Mat4f CameraComponent::CalcProjection() const {
    if (m_projection == ProjectionType::Orthographic) {
        const float half_height = m_ortho_height * 0.5f;
        const float half_width = half_height * aspect();
        return math::BuildOrthoRH(-half_width,
                                  half_width,
                                  -half_height,
                                  half_height,
                                  m_near,
                                  m_far);
    }
    return math::BuildPerspectiveRH(glm::radians<float>(m_fovy), aspect(), m_near, m_far);
}

Mat4f CameraComponent::CalcProjectionGL() const {
    if (m_projection == ProjectionType::Orthographic) {
        const float half_height = m_ortho_height * 0.5f;
        const float half_width = half_height * aspect();
        return math::BuildOpenGlOrthoRH(-half_width,
                                        half_width,
                                        -half_height,
                                        half_height,
                                        m_near,
                                        m_far);
    }
    return math::BuildOpenGlPerspectiveRH(glm::radians<float>(m_fovy), aspect(), m_near, m_far);
}

bool CameraComponent::update(const math::Mat4f& transform) {
    // @NOTE: the logic is wrong here,
    // if the transform has changed, the cache must update too
    if (dirty() || true) {
        setDirty(false);

        m_front = (transform * -Vec4f::UnitZ).xyz;
        m_right = (transform * Vec4f::UnitX).xyz;
        m_up = (transform * Vec4f::UnitY).xyz;
        m_position = (transform * Vec4f::UnitW).xyz;

        // @TODO: should be inverse of transform
        m_view_matrix = LookAtRh(m_position, m_position + m_front, Vec3f::UnitY);

        // use gl matrix for frustum culling
        m_projection_matrix = CalcProjectionGL();
        m_proj_view_matrix = m_projection_matrix * m_view_matrix;
        return true;
    }

    return false;
}

void CameraComponent::setAspect(float aspect) {
    if (aspect != aspect_) {
        setDirty();
        m_aspect = aspect;
    }
}

void CameraComponent::setFovy(float degree) {
    if (degree != m_fovy) {
        setDirty();
        m_fovy = degree;
    }
}

void CameraComponent::setNear(float near) {
    if (near != m_near) {
        setDirty();
        m_near = near;
    }
}

void CameraComponent::setFar(float far) {
    if (far != m_far) {
        setDirty();
        m_far = far;
    }
}

void CameraComponent::setProjectionType(ProjectionType projection) {
    if (projection != m_projection) {
        setDirty();
        m_projection = projection;
    }
}

void CameraComponent::setOrthoHeight(float ortho_height) {
    if (ortho_height != m_ortho_height) {
        setDirty();
        m_ortho_height = ortho_height;
    }
}

}  // namespace cave
