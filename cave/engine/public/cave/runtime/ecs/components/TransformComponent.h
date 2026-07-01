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
    math::Vec3f translation_;

    CAVE_PROP(editor = Rotation)
    math::Vec4f rotation_;

    CAVE_PROP(editor = Scale)
    math::Vec3f scale_;

    // Non-serialized attributes
    math::Mat4f world_;

    uint32_t flags_ = DirtyFlag;

public:
    TransformComponent();

    const math::Vec3f& translation() const { return translation_; }
    const math::Vec4f& rotation() const { return rotation_; }
    const math::Vec3f& scale() const { return scale_; }

    void setTranslation(const math::Vec3f& v);
    void setRotation(const math::Vec4f& v);
    void setScale(const math::Vec3f& v);

    const math::Mat4f& worldMatrix() const { return world_; }

    void setWorldMatrix(const math::Mat4f& v) { world_ = v; }

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

    bool dirty() const { return flags_ & DirtyFlag; }
    void setDirty(bool dirty = true);

    void OnDeserialized() { flags_ |= DirtyFlag; }
};

}  // namespace cave
