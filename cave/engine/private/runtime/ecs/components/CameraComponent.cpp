#include "cave/runtime/ecs/components/CameraComponent.h"

#include "engine/private/core/math/MatrixTransform.h"

namespace cave {

using math::Mat4f;
using math::Vec3f;
using math::Vec4f;

Mat4f CameraComponent::CalcProjection() const {
    if (projection_ == ProjectionType::Orthographic) {
        const float half_height = ortho_height_ * 0.5f;
        const float half_width = half_height * aspect();
        return math::BuildOrthoRH(-half_width,
                                  half_width,
                                  -half_height,
                                  half_height,
                                  near_,
                                  far_);
    }
    return math::BuildPerspectiveRH(glm::radians<float>(fovy_), aspect(), near_, far_);
}

Mat4f CameraComponent::CalcProjectionGL() const {
    if (projection_ == ProjectionType::Orthographic) {
        const float half_height = ortho_height_ * 0.5f;
        const float half_width = half_height * aspect();
        return math::BuildOpenGlOrthoRH(-half_width,
                                        half_width,
                                        -half_height,
                                        half_height,
                                        near_,
                                        far_);
    }
    return math::BuildOpenGlPerspectiveRH(glm::radians<float>(fovy_), aspect(), near_, far_);
}

bool CameraComponent::update(const math::Mat4f& transform) {
    // @NOTE: the logic is wrong here,
    // if the transform has changed, the cache must update too
    if (dirty() || true) {
        setDirty(false);

        front_ = (transform * -Vec4f::UnitZ).xyz;
        right_ = (transform * Vec4f::UnitX).xyz;
        up_ = (transform * Vec4f::UnitY).xyz;
        position_ = (transform * Vec4f::UnitW).xyz;

        // @TODO: should be inverse of transform
        view_matrix_ = LookAtRh(position_, position_ + front_, Vec3f::UnitY);

        // use gl matrix for frustum culling
        projection_matrix_ = CalcProjectionGL();
        projection_view_matrix_ = projection_matrix_ * view_matrix_;
        return true;
    }

    return false;
}

void CameraComponent::setAspect(float aspect) {
    if (aspect != aspect_) {
        setDirty();
        aspect_ = aspect;
    }
}

void CameraComponent::setFovy(float degree) {
    if (degree != fovy_) {
        setDirty();
        fovy_ = degree;
    }
}

void CameraComponent::setNear(float near) {
    if (near != near_) {
        setDirty();
        near_ = near;
    }
}

void CameraComponent::setFar(float far) {
    if (far != far_) {
        setDirty();
        far_ = far;
    }
}

void CameraComponent::setProjectionType(ProjectionType projection) {
    if (projection != projection_) {
        setDirty();
        projection_ = projection;
    }
}

void CameraComponent::setOrthoHeight(float ortho_height) {
    if (ortho_height != ortho_height_) {
        setDirty();
        ortho_height_ = ortho_height;
    }
}

}  // namespace cave
