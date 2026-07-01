#include "cave/core/math/Angle.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "engine/private/core/math/MatrixTransform.h"

namespace cave {

using namespace math;

TransformComponent::TransformComponent()
    : translation_{ 0 }
    , rotation_{ Vec4f::UnitW }
    , scale_{ 1 }
    , world_{ 1 } {
    setDirty();
}

void TransformComponent::setTranslation(const math::Vec3f& v) {
    setDirty();
    translation_ = v;
}

void TransformComponent::setRotation(const math::Vec4f& v) {
    setDirty();
    rotation_ = v;
}

void TransformComponent::setScale(const math::Vec3f& v) {
    setDirty();
    scale_ = v;
}

Mat4f TransformComponent::localMatrix() const {
    Mat4f rotationMatrix = glm::toMat4(Quaternion(rotation_.w, rotation_.x, rotation_.y, rotation_.z));
    Mat4f translationMatrix = cave::Translate(translation_);
    Mat4f scaleMatrix = cave::Scale(scale_);
    return translationMatrix * rotationMatrix * scaleMatrix;
}

bool TransformComponent::updateTransform() {
    if (dirty()) {
        setDirty(false);
        world_ = localMatrix();
        return true;
    }
    return false;
}

void TransformComponent::translate(const Vec3f& translation) {
    setDirty();
    translation_.x += translation.x;
    translation_.y += translation.y;
    translation_.z += translation.z;
}

void TransformComponent::translateX(float delta) {
    setDirty();
    translation_.x += delta;
}

void TransformComponent::translateY(float delta) {
    setDirty();
    translation_.y += delta;
}

void TransformComponent::translateZ(float delta) {
    setDirty();
    translation_.z += delta;
}

void TransformComponent::rotate(const Vec3f& euler) {
    setDirty();
    glm::quat quat(rotation_.w, rotation_.x, rotation_.y, rotation_.z);
    quat = glm::quat(glm::vec3(euler.x, euler.y, euler.z)) * quat;

    rotation_.x = quat.x;
    rotation_.y = quat.y;
    rotation_.z = quat.z;
    rotation_.w = quat.w;
}

void TransformComponent::scale(const Vec3f& scale) {
    setDirty();
    scale_.x *= scale.x;
    scale_.y *= scale.y;
    scale_.z *= scale.z;
}

void TransformComponent::setLocalTransform(const Mat4f& matrix) {
    setDirty();
    Decompose(matrix, scale_, rotation_, translation_);
}

void TransformComponent::matrixTransform(const Mat4f& matrix) {
    setDirty();
    Decompose(matrix * localMatrix(), scale_, rotation_, translation_);
}

void TransformComponent::setDirty(bool dirty) {
    if (dirty) {
        flags_ |= DirtyFlag;
    } else {
        flags_ &= ~DirtyFlag;
    }
}

}  // namespace cave
