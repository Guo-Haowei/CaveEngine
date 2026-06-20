#include "deserializer.h"

#include "cave/core/ids/Guid.h"
#include "cave/core/math/Angle.h"

namespace cave {

bool IDeserializer::Read(ecs::Entity& p_object) {
    uint32_t raw = 0;
    if (!Read(raw)) {
        return false;
    }

    p_object = ecs::Entity(raw);
    return true;
}

bool IDeserializer::Read(math::Degree& p_object) {
    float raw = 0;
    if (!Read(raw)) {
        return false;
    }

    p_object = math::Degree(raw);
    return true;
}

bool IDeserializer::Read(Guid& p_object) {
    std::string raw;
    if (!Read(raw)) {
        return false;
    }

    auto res = Guid::Parse(raw);

    ERR_FAIL_COND_V_MSG(res.is_none(), false, "failed to parse guid");

    p_object = res.unwrap_unchecked();
    return true;
}

bool IDeserializer::Read(math::Mat4f& p_object) {
    const auto size = ArraySize().unwrap_or(-1);
    ERR_FAIL_COND_V_MSG(size != 16, false, "expect float[16]");

    float* ptr = &p_object[0].x;
    for (int i = 0; i < 16; ++i) {
        TryEnterIndex(i);
        Read(ptr[i]);
        LeaveIndex();
    }

    return true;
}

Option<std::vector<std::string>> IDeserializer::GetKeys() {
    CRASH_NOW_MSG("GetKeys is very inefficient, try to avoid using it");
    std::exit(-1);
}

}  // namespace cave
