// =============================================================================
// File: cave/runtime/ecs/components/ColliderComponent.h
// =============================================================================
#pragma once
#include "cave/core/math/Vector.h"
#include "cave/runtime/ecs/ComponentDefines.h"

#define FLAG_GETTER_SETTER(FLAG, DATA)             \
    bool has##FLAG() const { return DATA & FLAG; } \
    void set##FLAG(bool value = true) { value ? DATA |= FLAG : DATA &= ~FLAG; }

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
    BodyType body_type_;

    CAVE_PROP(editor = Toggle, serialize = false)
    bool debug_draw_ = true;

    CAVE_PROP()
    Shape shape_;

    CAVE_PROP()
    uint32_t flags_ = None;

    CAVE_PROP()
    uint64_t category_ = 0;

    CAVE_PROP()
    uint64_t mask_ = 0;

    // Non-serialized
    mutable uint64_t user_data_ = 0;

    friend class Box2dPhysicsSystem;
    friend class Bullet3PhysicsManager;

public:
    FLAG_GETTER_SETTER(FixedRotationFlag, flags_)
    FLAG_GETTER_SETTER(SensorFlag, flags_)
    FLAG_GETTER_SETTER(BulletFlag, flags_)

    Shape& shape() { return shape_; }
    const Shape& shape() const { return shape_; }

    BodyType& bodyType() { return body_type_; }
    const BodyType& bodyType() const { return body_type_; }

    bool debugDraw() const { return debug_draw_; }
};

}  // namespace cave
