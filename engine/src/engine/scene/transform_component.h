#pragma once
#include "scene_component_base.h"

#include "engine/math/angle.h"
#include "engine/math/geomath.h"
#include "engine/reflection/reflection.h"

namespace cave {

class Archive;
class Degree;

class TransformComponent : public ComponentFlagBase {
    CAVE_META(TransformComponent)

    CAVE_PROP(editor = Scale)
    Vector3f m_scale;

    CAVE_PROP(editor = Translation)
    Vector3f m_translation;

    CAVE_PROP(editor = Rotation)
    Vector4f m_rotation;

private:
    // Non-serialized attributes
    Matrix4x4f m_world_matrix;

public:
    TransformComponent();

    const Vector3f& GetTranslation() const { return m_translation; }
    void SetTranslation(const Vector3f& p_translation) { m_translation = p_translation; }
    void IncreaseTranslation(const Vector3f& p_delta) { m_translation += p_delta; }

    const Vector4f& GetRotation() const { return m_rotation; }
    void SetRotation(const Vector4f& p_rotation) { m_rotation = p_rotation; }

    const Vector3f& GetScale() const { return m_scale; }
    void SetScale(const Vector3f& p_scale) { m_scale = p_scale; }

    const Matrix4x4f& GetWorldMatrix() const { return m_world_matrix; }

    void SetWorldMatrix(const Matrix4x4f& p_matrix) { m_world_matrix = p_matrix; }

    Matrix4x4f GetLocalMatrix() const;

    bool UpdateTransform();
    void Scale(const Vector3f& p_scale);
    void Translate(const Vector3f& p_translation);
    void Rotate(const Vector3f& p_euler);
    void RotateX(const Degree& p_degree) { Rotate(Vector3f(p_degree.GetRadians(), 0.0f, 0.0f)); }
    void RotateY(const Degree& p_degree) { Rotate(Vector3f(0.0f, p_degree.GetRadians(), 0.0f)); }
    void RotateZ(const Degree& p_degree) { Rotate(Vector3f(0.0f, 0.0f, p_degree.GetRadians())); }

    void SetLocalTransform(const Matrix4x4f& p_matrix);
    void MatrixTransform(const Matrix4x4f& p_matrix);

    void UpdateTransformParented(const TransformComponent& p_parent);

    void Serialize(Archive& p_archive, uint32_t p_version);
};

}  // namespace cave
