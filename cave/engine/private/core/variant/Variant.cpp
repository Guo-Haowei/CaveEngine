#include "cave/core/variant/Variant.h"

#include <format>

namespace cave {

using namespace ::cave::math;

Variant::Variant(VariantType type)
    : m_type(type) {
    std::memset(&m_ivec, 0, sizeof(m_ivec));
}

Variant::Variant(bool value)
    : m_type(VariantType::Int)
    , m_int(value ? 1 : 0) {}

Variant::Variant(int value)
    : m_type(VariantType::Int)
    , m_int(value) {}

Variant::Variant(float value)
    : m_type(VariantType::Float)
    , m_float(value) {}

Variant::Variant(const char* value)
    : m_type(VariantType::String)
    , m_int(0)
    , m_string(value) {}

Variant::Variant(std::string_view value)
    : m_type(VariantType::String)
    , m_int(0)
    , m_string(value) {}

Variant::Variant(std::string value)
    : m_type(VariantType::String)
    , m_int(0)
    , m_string(std::move(value)) {}

Variant::Variant(float x, float y)
    : m_type(VariantType::Vec2f)
    , m_vec{ x, y, 0.0f, 0.0f } {}

Variant::Variant(float x, float y, float z)
    : m_type(VariantType::Vec3f)
    , m_vec{ x, y, z, 0.0f } {}

Variant::Variant(float x, float y, float z, float w)
    : m_type(VariantType::Vec4f)
    , m_vec{ x, y, z, w } {}

Variant::Variant(int x, int y)
    : m_type(VariantType::Vec2i)
    , m_ivec{ x, y, 0, 0 } {}

Variant::Variant(int x, int y, int z)
    : m_type(VariantType::Vec3i)
    , m_ivec{ x, y, z, 0 } {}

Variant::Variant(int x, int y, int z, int w)
    : m_type(VariantType::Vec4i)
    , m_ivec{ x, y, z, w } {}

bool Variant::isNumeric() const {
    return m_type == VariantType::Int || m_type == VariantType::Float;
}

bool Variant::asBool(bool fallback) const {
    if (m_type == VariantType::Int) {
        return m_int != 0;
    }
    if (m_type == VariantType::Float) {
        return m_float != 0.0f;
    }
    return fallback;
}

int Variant::asInt(int fallback) const {
    if (m_type == VariantType::Int) {
        return m_int;
    }
    if (m_type == VariantType::Float) {
        return static_cast<int>(m_float);
    }
    return fallback;
}

float Variant::asFloat(float fallback) const {
    if (m_type == VariantType::Float) {
        return m_float;
    }
    if (m_type == VariantType::Int) {
        return static_cast<float>(m_int);
    }
    return fallback;
}

std::string_view Variant::asString(std::string_view fallback) const {
    return m_type == VariantType::String ? std::string_view(m_string) : fallback;
}

Vec2f Variant::asVec2f(Vec2f fallback) const {
    return m_type == VariantType::Vec2f ? m_vec.xy : fallback;
}

Vec3f Variant::asVec3f(Vec3f fallback) const {
    return m_type == VariantType::Vec3f ? m_vec.xyz : fallback;
}

Vec4f Variant::asVec4f(Vec4f fallback) const {
    return m_type == VariantType::Vec4f ? m_vec : fallback;
}

Vec2i Variant::asVec2i(Vec2i fallback) const {
    return m_type == VariantType::Vec2i ? m_ivec.xy : fallback;
}

Vec3i Variant::asVec3i(Vec3i fallback) const {
    return m_type == VariantType::Vec3i ? m_ivec.xyz : fallback;
}

Vec4i Variant::asVec4i(Vec4i fallback) const {
    return m_type == VariantType::Vec4i ? m_ivec : fallback;
}

void* Variant::asPointer() {
    switch (m_type) {
        case VariantType::Int:
        case VariantType::Float:
        case VariantType::Vec2f:
        case VariantType::Vec3f:
        case VariantType::Vec4f:
        case VariantType::Vec2i:
        case VariantType::Vec3i:
        case VariantType::Vec4i:
            return &m_int;
        default:
            CRASH_NOW();
            return nullptr;
    }
}

bool Variant::operator==(const Variant& rhs) const {
    if (m_type != rhs.m_type) {
        return false;
    }

    switch (m_type) {
        case VariantType::Invalid:
            return true;
        case VariantType::Int:
            return m_int == rhs.m_int;
        case VariantType::Float:
            return m_float == rhs.m_float;
        case VariantType::String:
            return m_string == rhs.m_string;
        case VariantType::Vec2f:
            return asVec2f() == rhs.asVec2f();
        case VariantType::Vec3f:
            return asVec3f() == rhs.asVec3f();
        case VariantType::Vec4f:
            return m_vec == rhs.m_vec;
        case VariantType::Vec2i:
            return asVec2i() == rhs.asVec2i();
        case VariantType::Vec3i:
            return asVec3i() == rhs.asVec3i();
        case VariantType::Vec4i:
            return m_ivec == rhs.m_ivec;
        default:
            return false;
    }
}

// @TODO: serializer?
std::string Variant::toString() const {
    switch (type()) {
        case VariantType::Int:
            return std::format("{}", m_int);
        case VariantType::Float:
            return std::format("{}", m_float);
        case VariantType::String:
            return std::format("\"{}\"", m_string);
        case VariantType::Vec2f:
            return std::format("{} {}", m_vec.x, m_vec.y);
        case VariantType::Vec2i:
            return std::format("{} {}", m_ivec.x, m_ivec.y);
        case VariantType::Vec3f:
            return std::format("{} {} {}", m_vec.x, m_vec.y, m_vec.z);
        case VariantType::Vec3i:
            return std::format("{} {} {}", m_ivec.x, m_ivec.y, m_ivec.z);
        case VariantType::Vec4f:
            return std::format("{} {} {} {}", m_vec.x, m_vec.y, m_vec.z, m_vec.w);
        case VariantType::Vec4i:
            return std::format("{} {} {} {}", m_ivec.x, m_ivec.y, m_ivec.z, m_ivec.w);
        default:
            CRASH_NOW();
            return std::string{};
    }
}

bool operator==(const VariantMap& lhs, const VariantMap& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (const auto& [key, lhs_value] : lhs) {
        auto it = rhs.find(key);
        if (it == rhs.end()) {
            return false;
        }

        if (lhs_value != it->second) {
            return false;
        }
    }

    return true;
}

bool operator!=(const VariantMap& lhs, const VariantMap& rhs) {
    return !(lhs == rhs);
}

}  // namespace cave
