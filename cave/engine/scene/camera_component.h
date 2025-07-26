#pragma once
#include "engine/math/angle.h"
#include "engine/math/geomath.h"
#include "engine/reflection/reflection.h"
#include "engine/scene/scene_component_base.h"

namespace cave {

class Degree;

class CameraComponent {
    enum : uint32_t {
        None = BIT(0),
        DirtyFlag = BIT(1),
        OrthoFlag = BIT(2),
        EditorFlag = BIT(3),
        PrimaryFlag = BIT(4),
        View2dFlag = BIT(5),
    };

    CAVE_META(CameraComponent)

    CAVE_PROP(type = flags)
    uint32_t m_flags = DirtyFlag;

    CAVE_PROP(type = degree)
    Degree m_fovy = DEFAULT_FOVY;

    CAVE_PROP(editor = DragFloat, min = 0.1f, max = 9)
    float m_near = DEFAULT_NEAR;

    CAVE_PROP(editor = DragFloat, min = 10, max = 10000)
    float m_far = DEFAULT_FAR;

    CAVE_PROP()
    int m_width = 0;

    CAVE_PROP()
    int m_height = 0;

    CAVE_PROP()
    float m_ortho_height = 10;

    CAVE_PROP()
    Degree m_pitch;

    CAVE_PROP()
    Degree m_yaw;

    CAVE_PROP()
    Vector3f m_position = Vector3f::Zero;

    // Not serlialized
    Vector3f m_front;
    Vector3f m_right;

    Matrix4x4f m_viewMatrix;
    Matrix4x4f m_projectionMatrix;
    Matrix4x4f m_projectionViewMatrix;

    friend class CameraControllerFPS;
    friend class EntityFactory;

public:
    static constexpr float DEFAULT_NEAR = 0.1f;
    static constexpr float DEFAULT_FAR = 1000.0f;
    static constexpr Degree DEFAULT_FOVY{ 50.0f };

    bool Update();

    void SetDimension(int p_width, int p_height);

    Degree GetFovy() const { return m_fovy; }
    void SetFovy(Degree p_degree) {
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

    const Vector3f& GetPosition() const { return m_position; }
    void SetPosition(const Vector3f& p_position);

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    float GetAspect() const { return (float)m_width / m_height; }

    float GetOrthoHeight() const { return m_ortho_height; }
    void SetOrthoHeight(float p_height);

    const Matrix4x4f& GetViewMatrix() const { return m_viewMatrix; }
    const Matrix4x4f& GetProjectionMatrix() const { return m_projectionMatrix; }
    const Matrix4x4f& GetProjectionViewMatrix() const { return m_projectionViewMatrix; }
    const Vector3f& GetRight() const { return m_right; }
    const Vector3f GetFront() const { return m_front; }

    Matrix4x4f CalcProjection() const;
    Matrix4x4f CalcProjectionGL() const;

    FLAG_GETTER_SETTER(DirtyFlag, m_flags)
    FLAG_GETTER_SETTER(OrthoFlag, m_flags)
    FLAG_GETTER_SETTER(EditorFlag, m_flags)
    FLAG_GETTER_SETTER(PrimaryFlag, m_flags)
    FLAG_GETTER_SETTER(View2dFlag, m_flags)

    void OnDeserialized() { m_flags |= DirtyFlag; }
};

}  // namespace cave
