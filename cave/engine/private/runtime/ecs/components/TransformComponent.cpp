#include "cave/core/math/Angle.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "engine/private/core/math/MatrixTransform.h"

namespace cave {

using namespace math;

TransformComponent::TransformComponent()
    : m_translation{ 0 }
    , m_rotation{ Vec4f::UnitW }
    , m_scale{ 1 }
    , m_world_matrix{ 1 } {
    SetDirty();
}

Mat4f TransformComponent::GetLocalMatrix() const {
    Mat4f rotationMatrix = glm::toMat4(Quaternion(m_rotation.w, m_rotation.x, m_rotation.y, m_rotation.z));
    Mat4f translationMatrix = cave::Translate(m_translation);
    Mat4f scaleMatrix = cave::Scale(m_scale);
    return translationMatrix * rotationMatrix * scaleMatrix;
}

bool TransformComponent::UpdateTransform() {
    if (IsDirty()) {
        SetDirty(false);
        m_world_matrix = GetLocalMatrix();
        return true;
    }
    return false;
}

void TransformComponent::Scale(const Vec3f& p_scale) {
    SetDirty();
    m_scale.x *= p_scale.x;
    m_scale.y *= p_scale.y;
    m_scale.z *= p_scale.z;
}

void TransformComponent::Translate(const Vec3f& p_translation) {
    SetDirty();
    m_translation.x += p_translation.x;
    m_translation.y += p_translation.y;
    m_translation.z += p_translation.z;
}

void TransformComponent::Rotate(const Vec3f& p_euler) {
    SetDirty();
    glm::quat quaternion(m_rotation.w, m_rotation.x, m_rotation.y, m_rotation.z);
    glm::quat euler(glm::vec3(p_euler.x, p_euler.y, p_euler.z));
    quaternion = euler * quaternion;

    m_rotation.x = quaternion.x;
    m_rotation.y = quaternion.y;
    m_rotation.z = quaternion.z;
    m_rotation.w = quaternion.w;
}

void TransformComponent::SetLocalTransform(const Mat4f& p_matrix) {
    SetDirty();
    Decompose(p_matrix, m_scale, m_rotation, m_translation);
}

void TransformComponent::MatrixTransform(const Mat4f& p_matrix) {
    SetDirty();
    Decompose(p_matrix * GetLocalMatrix(), m_scale, m_rotation, m_translation);
}

void TransformComponent::UpdateTransformParented(const TransformComponent& p_parent) {
    CRASH_NOW();
    Mat4f worldMatrix = GetLocalMatrix();
    const Mat4f& worldMatrixParent = p_parent.m_world_matrix;
    m_world_matrix = worldMatrixParent * worldMatrix;
}

void TransformComponent::SetDirty(bool p_dirty) {
    if (p_dirty) {
        m_flags |= DirtyFlag;
    } else {
        m_flags &= ~DirtyFlag;
    }
}

}  // namespace cave
