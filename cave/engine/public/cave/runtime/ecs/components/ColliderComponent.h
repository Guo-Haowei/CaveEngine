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
    BodyType body_type_;

    CAVE_PROP(editor = Toggle, serialize = false)
    bool debug_draw_ = true;

    CAVE_PROP()
    Shape shape_;

    CAVE_PROP()
    uint32_t flags_ = None;

    CAVE_PROP(editor = BitMask)
    uint32_t layer_ = 0;

    CAVE_PROP(editor = BitMask)
    uint32_t mask_ = 0;

    CAVE_PROP(editor = Toggle)
    bool is_trigger_ = false;

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

    uint32_t layer() const { return layer_; }
    void layer(uint32_t layer) { layer_ = layer; }

    uint32_t mask() const { return mask_; }
    void mask(uint32_t mask) { mask_ = mask; }

    bool isTrigger() const { return is_trigger_; }

    bool debugDraw() const { return debug_draw_; }
};

}  // namespace cave
