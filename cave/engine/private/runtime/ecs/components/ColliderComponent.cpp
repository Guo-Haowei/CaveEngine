#include "cave/runtime/ecs/components/ColliderComponent.h"

#include "engine/private/runtime/serialization/YamlInclude.h"

namespace cave {

using namespace cave::math;

Shape::Shape() {
    type = ShapeType::Null;
    data.half = Vec3f(0.5f);
}

Shape Shape::makeBox(const Vec2f& half) {
    Shape shape;
    shape.type = ShapeType::Box;
    shape.data.half.xy = half;
    return shape;
}

Shape Shape::makeBox(const Vec3f& half) {
    Shape shape;
    shape.type = ShapeType::Box;
    shape.data.half = half;
    return shape;
}

Shape Shape::makeRound(float radius) {
    Shape shape;
    shape.type = ShapeType::Round;
    shape.data.radius = radius;
    return shape;
}

ISerializer& WriteObject(ISerializer& s, const Shape& shape) {
    s.beginMap(false)
        .beginKey("type")
        .write(shape.type)
        .beginKey("data")
        .write(shape.data.half);
    s.endMap();
    return s;
}

bool ReadObject(IDeserializer& d, Shape& shape) {
    if (d.tryEnterKey("type")) {
        d.read(shape.type);
        d.leaveKey();
    }
    if (d.tryEnterKey("data")) {
        d.read(shape.data.half);
        d.leaveKey();
    }
    return true;
}

}  // namespace cave
