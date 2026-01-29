#pragma once
#include "cave/core/math/Angle.h"
#include "engine/private/math/geomath.h"
#include "engine/private/reflection/reflection.h"
#include "engine/private/runtime/scene/SceneComponentBase.h"

namespace cave::math {
class Degree;
}  // namespace cave::math

namespace cave {

enum class ProjectionType : uint8_t {
    Perspective,
    Orthographic,
    Count,
};

DECLARE_ENUM_TRAITS(ProjectionType, "perspective", "orthographic");

class CameraComponent {
    CAVE_META(CameraComponent)

    enum : uint32_t {
        None = 0b00,
        DirtyFlag = 0b1,
    };

private:
    CAVE_PROP()
    uint32_t m_flags = DirtyFlag;

    CAVE_PROP(editor = EnumDropDown)
    ProjectionType m_projection;

    CAVE_PROP(editor = DragFloat, min = 1, max = 179)
    float m_fovy = DEFAULT_FOVY;

    CAVE_PROP(editor = DragFloat, min = 0.1f, max = 9)
    float m_near = DEFAULT_NEAR;

    CAVE_PROP(editor = DragFloat, min = 10, max = 10000)
    float m_far = DEFAULT_FAR;

    CAVE_PROP(editor = InputInt)
    int m_width = 0;

    CAVE_PROP(editor = InputInt)
    int m_height = 0;

    CAVE_PROP()
    float m_ortho_height = 10;

    // Not serlialized
    math::Vector3f m_front = -math::Vector3f::UnitZ;
    math::Vector3f m_right = math::Vector3f::UnitX;
    math::Vector3f m_up = math::Vector3f::UnitY;
    math::Vector3f m_position = math::Vector3f::Zero;

    math::Matrix4x4f m_view_matrix;
    math::Matrix4x4f m_projection_matrix;
    math::Matrix4x4f m_projection_view_matrix;

    friend class CameraControllerFPS;
    friend class EntityFactory;

public:
    static constexpr float DEFAULT_NEAR = 0.1f;
    static constexpr float DEFAULT_FAR = 1000.0f;
    static constexpr float DEFAULT_FOVY = 60.0f;

    bool Update(const math::Matrix4x4f& p_transform);

    void SetDimension(int p_width, int p_height);

    float GetFovy() const { return m_fovy; }

    void SetFovy(float p_degree) {
        m_fovy = p_degree;
        SetDirtyFlag();
    }

    float GetNear() const { return m_near; }
    void SetNear(float p_near) {
        m_near = p_near;
        SetDirtyFlag();
    }

    float GetFar() const { return m_far; }
    void SetFar(float p_far) {
        m_far = p_far;
        SetDirtyFlag();
    }

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    float GetAspect() const { return (float)m_width / m_height; }

    void SetProjection(ProjectionType p_projection) { m_projection = p_projection; }

    float GetOrthoHeight() const { return m_ortho_height; }
    void SetOrthoHeight(float p_height);

    const math::Matrix4x4f& GetViewMatrix() const { return m_view_matrix; }
    const math::Matrix4x4f& GetProjectionMatrix() const { return m_projection_matrix; }
    const math::Matrix4x4f& GetProjectionViewMatrix() const { return m_projection_view_matrix; }
    const math::Vector3f& GetFront() const { return m_front; }
    const math::Vector3f& GetRight() const { return m_right; }
    const math::Vector3f& GetUp() const { return m_up; }
    const math::Vector3f& GetPosition() const { return m_position; }

    math::Matrix4x4f CalcProjection() const;
    math::Matrix4x4f CalcProjectionGL() const;

    FLAG_GETTER_SETTER(DirtyFlag, m_flags)

    void OnDeserialized() { m_flags |= DirtyFlag; }
};

}  // namespace cave
