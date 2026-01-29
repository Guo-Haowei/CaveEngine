#pragma once
#include "SceneComponentBase.h"

#include "cave/core/math/Angle.h"
#include "engine/private/core/math/geomath.h"
#include "engine/private/reflection/reflection.h"

namespace cave::math {
class Degree;
}  // namespace cave::math

namespace cave {

class TransformComponent : public ComponentFlagBase {
    CAVE_META(TransformComponent)

private:
    CAVE_PROP(editor = Translation)
    math::Vector3f m_translation;

    CAVE_PROP(editor = Rotation)
    math::Vector4f m_rotation;

    CAVE_PROP(editor = Scale)
    math::Vector3f m_scale;

    // Non-serialized attributes
    math::Matrix4x4f m_world_matrix;

public:
    TransformComponent();

    const math::Vector3f& GetTranslation() const { return m_translation; }
    void SetTranslation(const math::Vector3f& p_translation) { m_translation = p_translation; }
    void IncreaseTranslation(const math::Vector3f& p_delta) { m_translation += p_delta; }

    const math::Vector4f& GetRotation() const { return m_rotation; }
    void SetRotation(const math::Vector4f& p_rotation) { m_rotation = p_rotation; }

    const math::Vector3f& GetScale() const { return m_scale; }
    void SetScale(const math::Vector3f& p_scale) { m_scale = p_scale; }

    const math::Matrix4x4f& GetWorldMatrix() const { return m_world_matrix; }

    void SetWorldMatrix(const math::Matrix4x4f& p_matrix) { m_world_matrix = p_matrix; }

    math::Matrix4x4f GetLocalMatrix() const;

    bool UpdateTransform();
    void Scale(const math::Vector3f& p_scale);
    void Translate(const math::Vector3f& p_translation);
    void Rotate(const math::Vector3f& p_euler);
    void RotateX(const math::Degree& p_degree) { Rotate(math::Vector3f(p_degree.GetRadians(), 0.0f, 0.0f)); }
    void RotateY(const math::Degree& p_degree) { Rotate(math::Vector3f(0.0f, p_degree.GetRadians(), 0.0f)); }
    void RotateZ(const math::Degree& p_degree) { Rotate(math::Vector3f(0.0f, 0.0f, p_degree.GetRadians())); }

    void SetLocalTransform(const math::Matrix4x4f& p_matrix);
    void MatrixTransform(const math::Matrix4x4f& p_matrix);

    void UpdateTransformParented(const TransformComponent& p_parent);
};

}  // namespace cave
