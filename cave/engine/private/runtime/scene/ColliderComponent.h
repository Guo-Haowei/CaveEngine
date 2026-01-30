#pragma once
#include "engine/private/core/math/geomath.h"
#include "cave/core/reflection/Reflection.h"

#define FLAG_GETTER_SETTER(FLAG, DATA)             \
    bool Has##FLAG() const { return DATA & FLAG; } \
    void Set##FLAG(bool p_value = true) { p_value ? DATA |= FLAG : DATA &= ~FLAG; }

/*
[Entity Root]
    Collider (physics, movement)
        Animator (decides poses, plays clips, blends layers)
            SpriteRenderer (LowerBody)
            SpriteRenderer (UpperBody)
*/

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
        math::Vector3f half;  // box
        float radius;         // sphere, circle
    } data;

    Shape();

    static Shape MakeBox(const math::Vector2f& p_half);
    static Shape MakeBox(const math::Vector3f& p_half);
    static Shape MakeRound(float p_half);
};

ISerializer& WriteObject(ISerializer& s, const Shape& p_shape);

bool ReadObject(IDeserializer& d, Shape& p_shape);

class ColliderComponent {
    CAVE_META(ColliderComponent)

    enum : uint32_t {
        None = 0,
        FixedRotationFlag = BIT(0),
        SensorFlag = BIT(1),
        BulletFlag = BIT(2),
    };

private:
    CAVE_PROP(editor = EnumDropDown)
    BodyType m_body_type;

    CAVE_PROP(editor = Toggle, serialize = false)
    bool m_debug_draw = true;

    CAVE_PROP()
    Shape m_shape;

    CAVE_PROP()
    uint32_t m_flags = None;

    CAVE_PROP()
    uint64_t m_category = 0;

    CAVE_PROP()
    uint64_t m_mask = 0;

    // Non-serialized
    mutable uint64_t m_user_data = 0;

    friend class Box2dPhysicsManager;
    friend class Bullet3PhysicsManager;

public:
    FLAG_GETTER_SETTER(FixedRotationFlag, m_flags)
    FLAG_GETTER_SETTER(SensorFlag, m_flags)
    FLAG_GETTER_SETTER(BulletFlag, m_flags)

    Shape& GetShape() { return m_shape; }
    const Shape& GetShape() const { return m_shape; }

    BodyType& GetBodyType() { return m_body_type; }
    const BodyType& GetBodyType() const { return m_body_type; }

    bool GetDebugDraw() const { return m_debug_draw; }
};

}  // namespace cave
