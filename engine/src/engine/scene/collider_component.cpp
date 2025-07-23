#include "collider_component.h"

#include "engine/serialization/yaml_include.h"

namespace cave {

Shape::Shape() {
    type = ShapeType::Null;
    data.half = Vector3f(0.5f);
}

Shape Shape::MakeBox(const Vector2f& p_half) {
    Shape shape;
    shape.type = ShapeType::Box;
    shape.data.half.xy = p_half;
    return shape;
}

Shape Shape::MakeBox(const Vector3f& p_half) {
    Shape shape;
    shape.type = ShapeType::Box;
    shape.data.half = p_half;
    return shape;
}

Shape Shape::MakeRound(float p_radius) {
    Shape shape;
    shape.type = ShapeType::Round;
    shape.data.radius = p_radius;
    return shape;
}

ISerializer& WriteObject(ISerializer& s, const Shape& p_shape) {
    s.BeginMap(false)
        .Key("type")
        .Write(p_shape.type);
    s.EndMap();
    return s;
}

bool ReadObject(IDeserializer& d, Shape& p_shape) {
    if (d.TryEnterKey("type")) {
        d.Read(p_shape.type);
        d.LeaveKey();
    }
    return true;
}

}  // namespace cave
