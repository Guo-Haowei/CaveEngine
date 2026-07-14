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
    math::Mat4f m_world;

    uint32_t m_flags = DirtyFlag;

public:
    TransformComponent();

    const math::Vec3f& translation() const { return m_translation; }
    const math::Vec4f& rotation() const { return m_rotation; }
    const math::Vec3f& scale() const { return m_scale; }

    void setTranslation(const math::Vec3f& v);
    void setRotation(const math::Vec4f& v);
    void setScale(const math::Vec3f& v);

    const math::Mat4f& worldMatrix() const { return m_world; }

    void setWorldMatrix(const math::Mat4f& v) { m_world = v; }

    math::Mat4f localMatrix() const;

    bool updateTransform();

    void translate(const math::Vec3f& translation);
    void translateX(float delta);
    void translateY(float delta);
    void translateZ(float delta);

    void rotate(const math::Vec3f& euler);
    void rotateX(const math::Degree& degree) { rotate(math::Vec3f(degree.radians(), 0.0f, 0.0f)); }
    void rotateY(const math::Degree& degree) { rotate(math::Vec3f(0.0f, degree.radians(), 0.0f)); }
    void rotateZ(const math::Degree& degree) { rotate(math::Vec3f(0.0f, 0.0f, degree.radians())); }

    void scale(const math::Vec3f& scale);

    void setLocalTransform(const math::Mat4f& matrix);
    void matrixTransform(const math::Mat4f& matrix);

    bool dirty() const { return m_flags & DirtyFlag; }
    void setDirty(bool dirty = true);

    bool operator==(const TransformComponent& rhs) const {
        return translation() == rhs.translation() &&
               rotation() == rhs.rotation() &&
               scale() == rhs.scale();
    }

    bool operator!=(const TransformComponent& rhs) const {
        return !(this->operator==(rhs));
    }

    void onDeserialized() { m_flags |= DirtyFlag; }
};

}  // namespace cave
