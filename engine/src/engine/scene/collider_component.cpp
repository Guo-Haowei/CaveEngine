#include "collider_component.h"

#include "engine/serialization/yaml_include.h"

namespace cave {

Shape Shape::MakeNull() {
    Shape shape;
    shape.type = Null;
    return shape;
}

Shape Shape::MakeBox(const Vector2f& p_half) {
    Shape shape;
    shape.type = Box;
    shape.data.half = Vector3f(p_half, .5f);
    return shape;
}

Shape Shape::MakeBox(const Vector3f& p_half) {
    Shape shape;
    shape.type = Box;
    shape.data.half = p_half;
    return shape;
}

Shape Shape::MakeRound(float p_radius) {
    Shape shape;
    shape.type = Round;
    shape.data.radius = p_radius;
    return shape;
}

ISerializer& WriteObject(ISerializer& s, const Shape& p_shape) {
    unused(p_shape);
    CRASH_NOW();
    return s;
}

bool ReadObject(IDeserializer& d, Shape& p_shape) {
    unused(d);
    unused(p_shape);
    CRASH_NOW();
    return false;
}

}  // namespace cave
