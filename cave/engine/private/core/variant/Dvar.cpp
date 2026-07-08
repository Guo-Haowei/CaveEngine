#include "Dvar.h"

#if USING(ENABLE_DVAR)

namespace cave {

using namespace cave::math;

Dvar::Dvar(VariantType type, DvarFlags flags, const char* desc)
    : variant_(Variant::makeDefault(type))
    , desc_(desc)
    , flags_(flags) {
}

void Dvar::registerInt(std::string_view key, int value) {
    if (!(flags_ & DVAR_FLAG_OVERRIDDEN)) {
        variant_ = Variant(value);
    }
    registerDvar(key, this);
}

void Dvar::registerFloat(std::string_view key, float value) {
    if (!(flags_ & DVAR_FLAG_OVERRIDDEN)) {
        variant_ = Variant(value);
    }
    registerDvar(key, this);
}

void Dvar::registerString(std::string_view key, std::string_view value) {
    if (!(flags_ & DVAR_FLAG_OVERRIDDEN)) {
        variant_ = Variant(value);
    }
    registerDvar(key, this);
}

void Dvar::registerVec2f(std::string_view key, float x, float y) {
    if (!(flags_ & DVAR_FLAG_OVERRIDDEN)) {
        variant_ = Variant(Vec2f{ x, y });
    }
    registerDvar(key, this);
}

void Dvar::registerVec3f(std::string_view key, float x, float y, float z) {
    if (!(flags_ & DVAR_FLAG_OVERRIDDEN)) {
        variant_ = Variant(Vec3f{ x, y, z });
    }
    registerDvar(key, this);
}

void Dvar::registerVec4f(std::string_view key, float x, float y, float z, float w) {
    if (!(flags_ & DVAR_FLAG_OVERRIDDEN)) {
        variant_ = Variant(Vec4f{ x, y, z, w });
    }
    registerDvar(key, this);
}

void Dvar::registerVec2i(std::string_view key, int x, int y) {
    if (!(flags_ & DVAR_FLAG_OVERRIDDEN)) {
        variant_ = Variant(Vec2i{ x, y });
    }
    registerDvar(key, this);
}

void Dvar::registerVec3i(std::string_view key, int x, int y, int z) {
    if (!(flags_ & DVAR_FLAG_OVERRIDDEN)) {
        variant_ = Variant(Vec3i{ x, y, z });
    }
    registerDvar(key, this);
}

void Dvar::registerVec4i(std::string_view key, int x, int y, int z, int w) {
    if (!(flags_ & DVAR_FLAG_OVERRIDDEN)) {
        variant_ = Variant(Vec4i{ x, y, z, w });
    }
    registerDvar(key, this);
}

bool Dvar::setInt(int value) {
    ERR_FAIL_COND_V(type() != VariantType::Int, false);
    variant_ = Variant(value);
    return true;
}

bool Dvar::setFloat(float p_value) {
    ERR_FAIL_COND_V(type() != VariantType::Float, false);
    variant_ = Variant(p_value);
    return true;
}

bool Dvar::setString(const std::string& value) {
    ERR_FAIL_COND_V(type() != VariantType::String, false);
    variant_ = Variant(value);
    return true;
}

bool Dvar::setString(std::string_view value) {
    ERR_FAIL_COND_V(type() != VariantType::String, false);
    variant_ = Variant(value);
    return true;
}

bool Dvar::setVec2f(float x, float y) {
    ERR_FAIL_COND_V(type() != VariantType::Vec2f, false);
    variant_ = Variant(Vec2f{ x, y });
    return true;
}

bool Dvar::setVec3f(float x, float y, float z) {
    ERR_FAIL_COND_V(type() != VariantType::Vec3f, false);
    variant_ = Variant(Vec3f{ x, y, z });
    return true;
}

bool Dvar::setVec4f(float x, float y, float z, float w) {
    ERR_FAIL_COND_V(type() != VariantType::Vec4f, false);
    variant_ = Variant(Vec4f{ x, y, z, w });
    return true;
}

bool Dvar::setVec2i(int x, int y) {
    ERR_FAIL_COND_V(type() != VariantType::Vec2i, false);
    variant_ = Variant(Vec2i{ x, y });
    return true;
}

bool Dvar::setVec3i(int x, int y, int z) {
    ERR_FAIL_COND_V(type() != VariantType::Vec3i, false);
    variant_ = Variant(Vec3i{ x, y, z });
    return true;
}

bool Dvar::setVec4i(int x, int y, int z, int w) {
    ERR_FAIL_COND_V(type() != VariantType::Vec4i, false);
    variant_ = Variant(Vec4i{ x, y, z, w });
    return true;
}

Dvar* Dvar::findDvar(const std::string& p_name) {
    auto it = s_map.find(p_name);
    if (it == s_map.end()) {
        return nullptr;
    }
    return it->second;
}

void Dvar::registerDvar(std::string_view key, Dvar* dvar) {
    const std::string keyStr(key);
    auto it = s_map.find(keyStr);
    if (it != s_map.end()) {
        LOG_ERROR("duplicated dvar {} detected", key);
    }

    dvar->name_ = key;

    s_map.insert(std::make_pair(keyStr, dvar));
}

}  // namespace cave

#endif
