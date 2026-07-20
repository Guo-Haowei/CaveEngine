// =============================================================================
// File: cave/runtime/ecs/components/ColliderComponent.h
// =============================================================================
#pragma once
#include "cave/core/math/Vec.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

class ISerializer;
class IDeserializer;

enum class ShapeType : uint8_t {
    Null,
    Box,
    Round,
    Capsule,
    Count,
};

DECLARE_ENUM_TRAITS(ShapeType, "null", "box", "round", "capsule");

struct Shape {
    ShapeType type;

    union Data {
        math::Vec3f half;  // box
        float radius;      // sphere, circle
    } data;

    Shape();

    static Shape makeBox(const math::Vec2f& half);
    static Shape makeBox(const math::Vec3f& half);
    static Shape makeRound(float half);
};

ISerializer& WriteObject(ISerializer& s, const Shape& shape);

bool ReadObject(IDeserializer& d, Shape& shape);

class ColliderComponent {
    CAVE_COMPONENT(ColliderComponent)

private:
    CAVE_PROP()
    Shape m_shape;

    CAVE_PROP(editor = BitMask)
    uint32_t m_layer = 0;

    CAVE_PROP(editor = BitMask)
    uint32_t m_mask = 0;

    CAVE_PROP(editor = Toggle)
    bool m_is_trigger = false;

    // Non-serialized
    mutable uint64_t m_user_data = 0;

    friend class Box2dPhysicsSystem;
    friend class Bullet3PhysicsManager;

public:
    Shape& shape() { return m_shape; }
    const Shape& shape() const { return m_shape; }

    uint32_t layer() const { return m_layer; }
    void setLayer(uint32_t layer) { m_layer = layer; }

    uint32_t mask() const { return m_mask; }
    void setMask(uint32_t mask) { m_mask = mask; }

    bool isTrigger() const { return m_is_trigger; }
    void setTrigger(bool value = true) { m_is_trigger = value; }
};

}  // namespace cave
