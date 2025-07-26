#include "serializer.h"

namespace cave {

#if USING(VALIDATE_SERIALIZER)
void ISerializer::CheckEnter(SerializerState p_state) {
    m_stack.push_back(p_state);
}

void ISerializer::CheckExit(SerializerState p_state) {
    DEV_ASSERT(!m_stack.empty() && m_stack.back() == p_state);
    m_stack.pop_back();
}
#endif

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
