#include "transform_component.h"

#include "engine/math/angle.h"
#include "engine/math/matrix_transform.h"
#include "engine/core/io/archive.h"

// @TODO: remove
#include "engine/systems/serialization/serialization.h"

namespace cave {

TransformComponent::TransformComponent()
    : m_scale{ 1 }
    , m_translation{ 0 }
    , m_rotation{ Vector4f::UnitW }
    , m_world_matrix{ 1 } {
    SetDirty();
}

Matrix4x4f TransformComponent::GetLocalMatrix() const {
    Matrix4x4f rotationMatrix = glm::toMat4(Quaternion(m_rotation.w, m_rotation.x, m_rotation.y, m_rotation.z));
    Matrix4x4f translationMatrix = cave::Translate(m_translation);
    Matrix4x4f scaleMatrix = cave::Scale(m_scale);
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

void TransformComponent::Scale(const Vector3f& p_scale) {
    SetDirty();
    m_scale.x *= p_scale.x;
    m_scale.y *= p_scale.y;
    m_scale.z *= p_scale.z;
}

void TransformComponent::Translate(const Vector3f& p_translation) {
    SetDirty();
    m_translation.x += p_translation.x;
    m_translation.y += p_translation.y;
    m_translation.z += p_translation.z;
}

void TransformComponent::Rotate(const Vector3f& p_euler) {
    SetDirty();
    glm::quat quaternion(m_rotation.w, m_rotation.x, m_rotation.y, m_rotation.z);
    glm::quat euler(glm::vec3(p_euler.x, p_euler.y, p_euler.z));
    quaternion = euler * quaternion;

    m_rotation.x = quaternion.x;
    m_rotation.y = quaternion.y;
    m_rotation.z = quaternion.z;
    m_rotation.w = quaternion.w;
}

void TransformComponent::SetLocalTransform(const Matrix4x4f& p_matrix) {
    SetDirty();
    Decompose(p_matrix, m_scale, m_rotation, m_translation);
}

void TransformComponent::MatrixTransform(const Matrix4x4f& p_matrix) {
    SetDirty();
    Decompose(p_matrix * GetLocalMatrix(), m_scale, m_rotation, m_translation);
}

void TransformComponent::UpdateTransformParented(const TransformComponent& p_parent) {
    CRASH_NOW();
    Matrix4x4f worldMatrix = GetLocalMatrix();
    const Matrix4x4f& worldMatrixParent = p_parent.m_world_matrix;
    m_world_matrix = worldMatrixParent * worldMatrix;
}

void TransformComponent::Serialize(Archive& p_archive, uint32_t) {
    p_archive.ArchiveValue(flags);
    p_archive.ArchiveValue(m_scale);
    p_archive.ArchiveValue(m_translation);
    p_archive.ArchiveValue(m_rotation);
}

void TransformComponent::RegisterClass() {
}

}  // namespace cave
