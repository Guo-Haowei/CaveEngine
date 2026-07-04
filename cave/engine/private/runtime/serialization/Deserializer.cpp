#include "Deserializer.h"

#include "cave/core/ids/Guid.h"
#include "cave/core/math/Angle.h"
#include "cave/core/variant/Variant.h"

namespace cave {

bool IDeserializer::read(ecs::Entity& object) {
    uint32_t raw = 0;
    if (!read(raw)) {
        return false;
    }

    object = ecs::Entity(raw);
    return true;
}

bool IDeserializer::read(math::Degree& object) {
    float raw = 0;
    if (!read(raw)) {
        return false;
    }

    object = math::Degree(raw);
    return true;
}

bool IDeserializer::read(Guid& object) {
    std::string raw;
    if (!read(raw)) {
        return false;
    }

    auto res = Guid::parse(raw);

    ERR_FAIL_COND_V_MSG(res.is_none(), false, "failed to parse guid");

    object = res.unwrap_unchecked();
    return true;
}

bool IDeserializer::read(math::Mat4f& object) {
    const auto size = arraySize().unwrap_or(-1);
    ERR_FAIL_COND_V_MSG(size != 16, false, "expect float[16]");

    float* ptr = &object[0].x;
    for (int i = 0; i < 16; ++i) {
        tryEnterIndex(i);
        read(ptr[i]);
        leaveIndex();
    }

    return true;
}

bool IDeserializer::read(Variant& variant) {
    const auto size = arraySize().unwrap_or(-1);
    ERR_FAIL_COND_V_MSG(size != 16, false, "expect float[16]");

    VariantType type{};
    ERR_FAIL_COND_V_MSG(!tryEnterKey("type"), false, "expect type");
    read(type);
    leaveKey();

    DEV_ASSERT(type == VariantType::String);

    ERR_FAIL_COND_V_MSG(!tryEnterKey("value"), false, "expect value");
    std::string value;
    read(value);
    variant = Variant(value);
    leaveKey();

    return true;
}

Option<std::vector<std::string>> IDeserializer::getKeys() {
    CRASH_NOW_MSG("GetKeys is very inefficient, try to avoid using it");
    std::exit(-1);
}

}  // namespace cave
