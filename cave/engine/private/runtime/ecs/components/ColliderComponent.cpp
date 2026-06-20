#include "cave/runtime/ecs/components/ColliderComponent.h"

#include "engine/private/serialization/yaml_include.h"

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
    s.BeginMap(false)
        .Key("type")
        .Write(shape.type)
        .Key("data")
        .Write(shape.data.half);
    s.EndMap();
    return s;
}

bool ReadObject(IDeserializer& d, Shape& shape) {
    if (d.TryEnterKey("type")) {
        d.Read(shape.type);
        d.LeaveKey();
    }
    if (d.TryEnterKey("data")) {
        d.Read(shape.data.half);
        d.LeaveKey();
    }
    return true;
}

}  // namespace cave
