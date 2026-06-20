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
    uint32_t m_flags = DirtyFlag;

    CAVE_PROP(editor = EnumDropDown)
    ProjectionType m_projection;

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
    math::Mat4f m_projection_matrix;
    math::Mat4f m_projection_view_matrix;

public:
    static constexpr float kDefaultNear = 0.1f;
    static constexpr float kDefaultFar = 1000.0f;
    static constexpr float kDefaultFovy = 60.0f;

    bool Update(const math::Mat4f& p_transform);

    float GetFovy() const { return m_fovy; }

    void SetFovy(float p_degree) {
        m_fovy = p_degree;
        SetDirty();
    }

    float GetNear() const { return m_near; }
    void SetNear(float p_near) {
        m_near = p_near;
        SetDirty();
    }

    float GetFar() const { return m_far; }
    void SetFar(float p_far) {
        m_far = p_far;
        SetDirty();
    }

    float GetAspect() const { return m_aspect; }
    void SetAspect(float p_aspect) {
        m_aspect = p_aspect;
        SetDirty();
    }

    void SetProjection(ProjectionType p_projection) { m_projection = p_projection; }

    float GetOrthoHeight() const { return m_ortho_height; }
    void SetOrthoHeight(float p_height);

    math::Mat4f CalcProjection() const;
    math::Mat4f CalcProjectionGL() const;

    // ---------------- Accessors ----------------
    // these values are modified by Update() function only
    const math::Mat4f& GetViewMatrix() const { return m_view_matrix; }
    const math::Mat4f& GetProjectionMatrix() const { return m_projection_matrix; }
    const math::Mat4f& GetProjectionViewMatrix() const { return m_projection_view_matrix; }

    const math::Vec3f& GetFront() const { return m_front; }
    const math::Vec3f& GetRight() const { return m_right; }
    const math::Vec3f& GetUp() const { return m_up; }
    const math::Vec3f& GetPosition() const { return m_position; }

    bool IsDirty() const { return m_flags & DirtyFlag; }
    void SetDirty(bool p_value = true) { p_value ? m_flags |= DirtyFlag : m_flags &= ~DirtyFlag; }

    void OnDeserialized() { m_flags |= DirtyFlag; }
};

}  // namespace cave
