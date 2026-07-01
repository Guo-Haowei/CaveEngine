// =============================================================================
// File: cave/runtime/ecs/components/CameraComponent.h
// =============================================================================
#pragma once
#include "cave/core/math/Vector.h"
#include "cave/core/math/Matrix.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

enum class ProjectionType : uint8_t {
    Perspective,
    Orthographic,
    Count,
};

DECLARE_ENUM_TRAITS(ProjectionType, "perspective", "orthographic");

class CameraComponent {
    CAVE_COMPONENT(CameraComponent)

    enum Flags : uint32_t {
        None = 0,
        DirtyFlag = 1,
    };

private:
    CAVE_PROP()
    uint32_t flags_ = DirtyFlag;

    CAVE_PROP(editor = EnumDropDown)
    ProjectionType projection_;

    CAVE_PROP(editor = DragFloat, min = 1, max = 179)
    float fovy_ = kDefaultFovy;

    CAVE_PROP(editor = DragFloat, min = 0.1f, max = 9)
    float near_ = kDefaultNear;

    CAVE_PROP(editor = DragFloat, min = 10, max = 10000)
    float far_ = kDefaultFar;

    CAVE_PROP(editor = InputFloat)
    float aspect_ = 1.0f;

    CAVE_PROP()
    float ortho_height_ = 10;

    // Not serlialized
    math::Vec3f front_ = -math::Vec3f::UnitZ;
    math::Vec3f right_ = math::Vec3f::UnitX;
    math::Vec3f up_ = math::Vec3f::UnitY;
    math::Vec3f position_ = math::Vec3f::Zero;

    math::Mat4f view_matrix_;
    math::Mat4f projection_matrix_;
    math::Mat4f projection_view_matrix_;

public:
    static constexpr float kDefaultNear = 0.1f;
    static constexpr float kDefaultFar = 1000.0f;
    static constexpr float kDefaultFovy = 60.0f;

    bool update(const math::Mat4f& transform);

    float aspect() const { return aspect_; }
    void setAspect(float aspect);

    float fovy() const { return fovy_; }
    void setFovy(float degree);

    float near() const { return near_; }
    void setNear(float near);

    float far() const { return far_; }
    void setFar(float far);

    void setProjectionType(ProjectionType projection);

    float orthoHeight() const { return ortho_height_; }
    void setOrthoHeight(float height);

    math::Mat4f CalcProjection() const;
    math::Mat4f CalcProjectionGL() const;

    // ---------------- Accessors ----------------
    // these values are modified by Update() function only
    const math::Mat4f& viewMatrix() const { return view_matrix_; }
    const math::Mat4f& projectionMatrix() const { return projection_matrix_; }
    const math::Mat4f& projectionViewMatrix() const { return projection_view_matrix_; }

    const math::Vec3f& front() const { return front_; }
    const math::Vec3f& right() const { return right_; }
    const math::Vec3f& up() const { return up_; }
    const math::Vec3f& position() const { return position_; }

    bool dirty() const { return flags_ & DirtyFlag; }
    void setDirty(bool dirty = true) { dirty ? flags_ |= DirtyFlag : flags_ &= ~DirtyFlag; }

    void OnDeserialized() { flags_ |= DirtyFlag; }
};

}  // namespace cave
