// =============================================================================
// File: cave/runtime/ecs/components/TransformComponent.h
// =============================================================================
#pragma once
#include "cave/core/math/Angle.h"
#include "cave/core/math/Matrix.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave::math {
class Degree;
}  // namespace cave::math

namespace cave {

class TransformComponent {
    CAVE_COMPONENT(TransformComponent)

    enum Flags : uint32_t {
        None = 0,
        DirtyFlag = 1,
    };

private:
    CAVE_PROP(editor = Translation)
    math::Vec3f m_translation;

    CAVE_PROP(editor = Rotation)
    math::Vec4f m_rotation;

    CAVE_PROP(editor = Scale)
    math::Vec3f m_scale;

    // Non-serialized attributes
    math::Mat4f m_world_matrix;

    uint32_t m_flags = DirtyFlag;

public:
    TransformComponent();

    const math::Vec3f& GetTranslation() const { return m_translation; }
    void SetTranslation(const math::Vec3f& p_translation) { m_translation = p_translation; }
    void IncreaseTranslation(const math::Vec3f& p_delta) { m_translation += p_delta; }

    const math::Vec4f& GetRotation() const { return m_rotation; }
    void SetRotation(const math::Vec4f& p_rotation) { m_rotation = p_rotation; }

    const math::Vec3f& GetScale() const { return m_scale; }
    void SetScale(const math::Vec3f& p_scale) { m_scale = p_scale; }

    const math::Mat4f& GetWorldMatrix() const { return m_world_matrix; }

    void SetWorldMatrix(const math::Mat4f& p_matrix) { m_world_matrix = p_matrix; }

    math::Mat4f GetLocalMatrix() const;

    bool UpdateTransform();
    void Scale(const math::Vec3f& p_scale);
    void Translate(const math::Vec3f& p_translation);
    void Rotate(const math::Vec3f& p_euler);
    void RotateX(const math::Degree& degree) { Rotate(math::Vec3f(degree.radians(), 0.0f, 0.0f)); }
    void RotateY(const math::Degree& degree) { Rotate(math::Vec3f(0.0f, degree.radians(), 0.0f)); }
    void RotateZ(const math::Degree& degree) { Rotate(math::Vec3f(0.0f, 0.0f, degree.radians())); }

    void SetLocalTransform(const math::Mat4f& p_matrix);
    void MatrixTransform(const math::Mat4f& p_matrix);

    void UpdateTransformParented(const TransformComponent& p_parent);

    bool IsDirty() const { return m_flags & DirtyFlag; }
    void SetDirty(bool p_dirty = true);
    void OnDeserialized() { m_flags |= DirtyFlag; }
};

}  // namespace cave
