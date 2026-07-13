#include "cave/core/math/Angle.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "engine/private/core/math/MatrixTransform.h"

namespace cave {

using namespace math;

TransformComponent::TransformComponent()
    : m_translation{ 0 }
    , m_rotation{ Vec4f::UnitW }
    , m_scale{ 1 }
    , m_world{ 1 } {
    setDirty();
}

void TransformComponent::setTranslation(const math::Vec3f& v) {
    setDirty();
    m_translation = v;
}

void TransformComponent::setRotation(const math::Vec4f& v) {
    setDirty();
    m_rotation = v;
}

void TransformComponent::setScale(const math::Vec3f& v) {
    setDirty();
    m_scale = v;
}

Mat4f TransformComponent::localMatrix() const {
    Mat4f rotationMatrix = glm::toMat4(Quaternion(m_rotation.w, m_rotation.x, m_rotation.y, m_rotation.z));
    Mat4f translationMatrix = cave::Translate(m_translation);
    Mat4f scaleMatrix = cave::Scale(m_scale);
    return translationMatrix * rotationMatrix * scaleMatrix;
}

bool TransformComponent::updateTransform() {
    if (dirty()) {
        setDirty(false);
        m_world = localMatrix();
        return true;
    }
    return false;
}

void TransformComponent::translate(const Vec3f& translation) {
    setDirty();
    m_translation.x += translation.x;
    m_translation.y += translation.y;
    m_translation.z += translation.z;
}

void TransformComponent::translateX(float delta) {
    setDirty();
    m_translation.x += delta;
}

void TransformComponent::translateY(float delta) {
    setDirty();
    m_translation.y += delta;
}

void TransformComponent::translateZ(float delta) {
    setDirty();
    m_translation.z += delta;
}

void TransformComponent::rotate(const Vec3f& euler) {
    setDirty();
    glm::quat quat(m_rotation.w, m_rotation.x, m_rotation.y, m_rotation.z);
    quat = glm::quat(glm::vec3(euler.x, euler.y, euler.z)) * quat;

    m_rotation.x = quat.x;
    m_rotation.y = quat.y;
    m_rotation.z = quat.z;
    m_rotation.w = quat.w;
}

void TransformComponent::scale(const Vec3f& scale) {
    setDirty();
    m_scale.x *= scale.x;
    m_scale.y *= scale.y;
    m_scale.z *= scale.z;
}

void TransformComponent::setLocalTransform(const Mat4f& matrix) {
    setDirty();
    Decompose(matrix, m_scale, m_rotation, m_translation);
}

void TransformComponent::matrixTransform(const Mat4f& matrix) {
    setDirty();
    Decompose(matrix * localMatrix(), m_scale, m_rotation, m_translation);
}

void TransformComponent::setDirty(bool dirty) {
    if (dirty) {
        m_flags |= DirtyFlag;
    } else {
        m_flags &= ~DirtyFlag;
    }
}

}  // namespace cave
