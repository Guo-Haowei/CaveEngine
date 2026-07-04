#include "Serializer.h"

#include "cave/core/variant/Variant.h"

namespace cave {

#if USING(VALIDATE_SERIALIZER)
void ISerializer::checkEnter(SerializerState state) {
    stack_.push_back(state);
}

void ISerializer::checkExit(SerializerState state) {
    DEV_ASSERT(!stack_.empty() && stack_.back() == state);
    stack_.pop_back();
}
#endif

ISerializer& ISerializer::write(const ecs::Entity& object) {
    return write(object.GetId());
}

ISerializer& ISerializer::write(const math::Degree& object) {
    return write(object.degrees());
}

ISerializer& ISerializer::write(const math::Mat4f& object) {
    beginArray(true);
    const float* ptr = &object[0].x;
    for (int i = 0; i < 16; ++i) {
        write(ptr[i]);
    }
    endArray();
    return *this;
}

ISerializer& ISerializer::write(const Variant& variant) {
    const VariantType type = variant.type();
    beginMap(false)
        .beginKey("type")
        .write(type)
        .beginKey("value");

    switch (type) {
        case VariantType::String: {
            write(variant.asString());
        } break;
        default: {
            CRASH_NOW_MSG("not supported");
        } break;
    }

    endMap();
    return *this;
}

}  // namespace cave
