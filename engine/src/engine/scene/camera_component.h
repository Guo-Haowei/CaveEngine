#pragma once
#include "engine/math/angle.h"
#include "engine/math/geomath.h"
#include "engine/reflection/reflection.h"

namespace cave {

class Degree;

#define CAMERA_FLAG_LIST \
    CAMERA_FLAG(Dirty)   \
    CAMERA_FLAG(Ortho)   \
    CAMERA_FLAG(Editor)  \
    CAMERA_FLAG(Primary) \
    CAMERA_FLAG(View2D)

class CameraComponent {
    enum : uint32_t {
#define CAMERA_FLAG(FLAG) _##FLAG##_BIT,
        CAMERA_FLAG_LIST
#undef CAMERA_FLAG
            Count,
    };

    enum CameraFlags : uint32_t {
#define CAMERA_FLAG(FLAG) FLAG = 1u << _##FLAG##_BIT,
        CAMERA_FLAG_LIST
#undef CAMERA_FLAG
    };

    CAVE_META(CameraComponent)

    CAVE_PROP(type = flags)
    uint32_t m_flags = Dirty;

    CAVE_PROP(type = degree)
    Degree m_fovy = DEFAULT_FOVY;

    CAVE_PROP(type = f32)
    float m_near = DEFAULT_NEAR;

    CAVE_PROP(type = f32)
    float m_far = DEFAULT_FAR;

    CAVE_PROP(type = i32)
    int m_width = 0;

    CAVE_PROP(type = i32)
    int m_height = 0;

    CAVE_PROP(type = f32)
    float m_ortho_height = 10;

    CAVE_PROP(type = degree)
    Degree m_pitch;

    CAVE_PROP(type = degree)
    Degree m_yaw;

    CAVE_PROP(type = position)
    Vector3f m_position = Vector3f::Zero;

private:
    // Not serlialized
    Vector3f m_front;
    Vector3f m_right;

    Matrix4x4f m_viewMatrix;
    Matrix4x4f m_projectionMatrix;
    Matrix4x4f m_projectionViewMatrix;

public:
    static constexpr float DEFAULT_NEAR = 0.1f;
    static constexpr float DEFAULT_FAR = 1000.0f;
    static constexpr Degree DEFAULT_FOVY{ 50.0f };

    bool Update();

    void SetDimension(int p_width, int p_height);

    Degree GetFovy() const { return m_fovy; }
    void SetFovy(Degree p_degree) {
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

#define CAMERA_FLAG(FLAG)                            \
    bool Is##FLAG() const { return m_flags & FLAG; } \
    void Set##FLAG(bool p_flag = true) { p_flag ? m_flags |= FLAG : m_flags &= ~FLAG; }
    CAMERA_FLAG_LIST
#undef CAMERA_FLAG

    void OnDeserialized() { m_flags |= Dirty; }

private:
    friend class CameraControllerFPS;
    friend class EntityFactory;
};

}  // namespace cave
