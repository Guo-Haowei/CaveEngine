// =============================================================================
// File: cave/runtime/ecs/components/ColliderComponent.h
// =============================================================================
#pragma once
#include "cave/core/math/Vec.h"
#include "cave/runtime/ecs/ComponentDefines.h"

#define FLAG_GETTER_SETTER(FLAG, DATA)             \
    bool has##FLAG() const { return DATA & FLAG; } \
    void set##FLAG(bool value = true) { value ? DATA |= FLAG : DATA &= ~FLAG; }

namespace cave {

class ISerializer;
class IDeserializer;

enum class BodyType : uint8_t {
    Static = 0,
    Kinematic,
    Dynamic,
    Count,
};

DECLARE_ENUM_TRAITS(BodyType, "static", "kinematic", "dynamic");

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

    enum : uint32_t {
        None = 0,
        FixedRotationFlag = 1,
        SensorFlag = 2,
        BulletFlag = 4,
    };

private:
    CAVE_PROP(editor = EnumDropDown)
    BodyType m_body_type;

    CAVE_PROP()
    Shape m_shape;

    CAVE_PROP()
    uint32_t m_flags = None;

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
    FLAG_GETTER_SETTER(FixedRotationFlag, m_flags)
    FLAG_GETTER_SETTER(SensorFlag, m_flags)
    FLAG_GETTER_SETTER(BulletFlag, m_flags)

    Shape& shape() { return m_shape; }
    const Shape& shape() const { return m_shape; }

    BodyType& bodyType() { return m_body_type; }
    const BodyType& bodyType() const { return m_body_type; }

    uint32_t layer() const { return m_layer; }
    void layer(uint32_t layer) { m_layer = layer; }

    uint32_t mask() const { return m_mask; }
    void mask(uint32_t mask) { m_mask = mask; }

    bool isTrigger() const { return m_is_trigger; }
};

}  // namespace cave
