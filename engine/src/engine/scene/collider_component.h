#pragma once
#include "engine/math/geomath.h"
#include "engine/reflection/reflection.h"
#include "engine/scene/scene_component_base.h"

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
        Vector3f half;  // box
        float radius;   // sphere, circle
    } data;

    Shape();

    static Shape MakeBox(const Vector2f& p_half);
    static Shape MakeBox(const Vector3f& p_half);
    static Shape MakeRound(float p_half);
};

ISerializer& WriteObject(ISerializer& s, const Shape& p_shape);

bool ReadObject(IDeserializer& d, Shape& p_shape);

class ColliderComponent {
    enum : uint32_t {
        None = BIT(0),
        FixedRotationFlag = BIT(1),
        SensorFlag = BIT(2),
        BulletFlag = BIT(3),
    };

    CAVE_META(ColliderComponent)

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

    void OnDeserialized() {}
};

}  // namespace cave
