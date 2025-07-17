#include "serializer.h"

namespace cave {

ISerializer& ISerializer::Write(const ecs::Entity& p_object) {
    return Write(p_object.GetId());
}

ISerializer& ISerializer::Write(const Degree& p_object) {
    return Write(p_object.GetDegree());
}

ISerializer& ISerializer::Write(const Matrix4x4f& p_object) {
    BeginArray(true);
    const float* ptr = &p_object[0].x;
    for (int i = 0; i < 16; ++i) {
        Write(ptr[i]);
    }
    EndArray();
    return *this;
}

}  // namespace cave
