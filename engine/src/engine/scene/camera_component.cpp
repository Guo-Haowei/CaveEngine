#include "camera_component.h"

#include "engine/math/matrix_transform.h"

namespace cave {

Matrix4x4f CameraComponent::CalcProjection() const {
    if (IsOrtho()) {
        const float half_height = m_ortho_height * 0.5f;
        const float half_width = half_height * GetAspect();
        return BuildOrthoRH(-half_width,
                            half_width,
                            -half_height,
                            half_height,
                            m_near,
                            m_far);
    }
    return BuildPerspectiveRH(m_fovy.GetRadians(), GetAspect(), m_near, m_far);
}

Matrix4x4f CameraComponent::CalcProjectionGL() const {
    if (IsOrtho()) {
        const float half_height = m_ortho_height * 0.5f;
        const float half_width = half_height * GetAspect();
        return BuildOpenGlOrthoRH(-half_width,
                                  half_width,
                                  -half_height,
                                  half_height,
                                  m_near,
                                  m_far);
    }
    return BuildOpenGlPerspectiveRH(m_fovy.GetRadians(), GetAspect(), m_near, m_far);
}

bool CameraComponent::Update() {
    if (IsDirty()) {
        SetDirty(false);

        m_front.x = m_yaw.Sin() * m_pitch.Cos();
        m_front.y = m_pitch.Sin();
        m_front.z = m_yaw.Cos() * -m_pitch.Cos();

        m_right = cross(m_front, Vector3f::UnitY);

        m_viewMatrix = LookAtRh(m_position, m_position + m_front, Vector3f::UnitY);

        // use gl matrix for frustum culling
        m_projectionMatrix = CalcProjectionGL();
        m_projectionViewMatrix = m_projectionMatrix * m_viewMatrix;
        return true;
    }

    return false;
}

// @TODO: reflection?
void CameraComponent::SetDimension(int p_width, int p_height) {
    if (m_width != p_width || m_height != p_height) {
        m_width = p_width;
        m_height = p_height;
        SetDirty();
    }
}

void CameraComponent::SetOrthoHeight(float p_height) {
    if (p_height != m_ortho_height) {
        m_ortho_height = p_height;
        SetDirty();
    }
}

void CameraComponent::SetPosition(const Vector3f& p_position) {
    if (p_position != m_position) {
        m_position = p_position;
        SetDirty();
    }
}

}  // namespace cave
