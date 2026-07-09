// =============================================================================
// File: cave/runtime/ecs/components/CameraComponent.h
// =============================================================================
#pragma once
#include "cave/core/math/Vec.h"
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
    uint32_t m_flags = DirtyFlag;

    CAVE_PROP(editor = EnumDropDown)
    ProjectionType m_projection = ProjectionType::Perspective;

    CAVE_PROP(editor = DragFloat, min = 1, max = 179)
    float m_fovy = kDefaultFovy;

    CAVE_PROP(editor = DragFloat, min = 0.1f, max = 9)
    float m_near = kDefaultNear;

    CAVE_PROP(editor = DragFloat, min = 10, max = 10000)
    float m_far = kDefaultFar;

    CAVE_PROP(editor = InputFloat)
    float m_aspect = 1.0f;

    CAVE_PROP()
    float m_ortho_height = 10;

    // Not serlialized
    math::Vec3f m_front = -math::Vec3f::UnitZ;
    math::Vec3f m_right = math::Vec3f::UnitX;
    math::Vec3f m_up = math::Vec3f::UnitY;
    math::Vec3f m_position = math::Vec3f::Zero;

    math::Mat4f m_view_matrix;
    math::Mat4f m_proj_matrix;
    math::Mat4f m_proj_view_matrix;

public:
    static constexpr float kDefaultNear = 0.1f;
    static constexpr float kDefaultFar = 1000.0f;
    static constexpr float kDefaultFovy = 60.0f;

    bool update(const math::Mat4f& transform);

    float aspect() const { return m_aspect; }
    void setAspect(float aspect);

    float fovy() const { return m_fovy; }
    void setFovy(float degree);

    float near() const { return m_near; }
    void setNear(float near);

    float far() const { return m_far; }
    void setFar(float far);

    void setProjectionType(ProjectionType projection);

    float orthoHeight() const { return m_ortho_height; }
    void setOrthoHeight(float height);

    math::Mat4f CalcProjection() const;
    math::Mat4f CalcProjectionGL() const;

    // ---------------- Accessors ----------------
    // these values are modified by Update() function only
    const math::Mat4f& viewMatrix() const { return m_view_matrix; }
    const math::Mat4f& projectionMatrix() const { return m_proj_matrix; }
    const math::Mat4f& projectionViewMatrix() const { return m_proj_view_matrix; }

    const math::Vec3f& front() const { return m_front; }
    const math::Vec3f& right() const { return m_right; }
    const math::Vec3f& up() const { return m_up; }
    const math::Vec3f& position() const { return m_position; }

    bool dirty() const { return m_flags & DirtyFlag; }
    void setDirty(bool dirty = true) { dirty ? m_flags |= DirtyFlag : m_flags &= ~DirtyFlag; }

    void OnDeserialized() { m_flags |= DirtyFlag; }
};

}  // namespace cave
