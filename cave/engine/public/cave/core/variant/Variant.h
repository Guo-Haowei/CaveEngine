// =============================================================================
// File: cave/core/variant/Variant.h
// =============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

#include "cave/core/math/Vector.h"
#include "cave/core/reflection/Reflection.h"

namespace cave {

enum class VariantType : uint8_t {
    Invalid = 0,

    Int,
    Float,
    String,
    Vec2f,
    Vec3f,
    Vec4f,
    Vec2i,
    Vec3i,
    Vec4i,

    Count,
};

DECLARE_ENUM_TRAITS(VariantType,
                    "invalid",
                    "int", "float",
                    "string",
                    "vec2f", "vec3f", "vec4f",
                    "ivec2f", "ivec3f", "ivec4f");

class Variant {
public:
    Variant() = default;

    Variant(bool value);
    Variant(int value);
    Variant(float value);
    Variant(const char* value);
    Variant(std::string_view value);
    Variant(std::string value);
    Variant(const math::Vec2f& value);
    Variant(const math::Vec3f& value);
    Variant(const math::Vec4f& value);
    Variant(const math::Vec2i& value);
    Variant(const math::Vec3i& value);
    Variant(const math::Vec4i& value);

    VariantType type() const { return type_; }

    bool isValid() const { return type_ != VariantType::Invalid; }
    bool isNumeric() const;

    bool asBool(bool fallback = false) const;
    int asInt(int fallback = 0) const;
    float asFloat(float fallback = 0.0f) const;
    std::string_view asString(std::string_view fallback = {}) const;
    math::Vec2f asVec2f(math::Vec2f fallback = math::Vec2f::Zero) const;
    math::Vec3f asVec3f(math::Vec3f fallback = math::Vec3f::Zero) const;
    math::Vec4f asVec4f(math::Vec4f fallback = math::Vec4f::Zero) const;
    math::Vec2i asVec2i(math::Vec2i fallback = math::Vec2i::Zero) const;
    math::Vec3i asVec3i(math::Vec3i fallback = math::Vec3i::Zero) const;
    math::Vec4i asVec4i(math::Vec4i fallback = math::Vec4i::Zero) const;
    void* asPointer();

    std::string toString() const;
    
    bool operator==(const Variant& rhs) const;
    bool operator!=(const Variant& rhs) const { return !(*this == rhs); }

    static Variant makeDefault(VariantType type);

private:
    VariantType type_{};

    union {
        int int_;
        float float_;
        math::Vec4f vec_;
        math::Vec4i ivec_;
    };
    std::string string_;
};

using VariantMap = std::unordered_map<std::string, Variant>;

bool operator==(const VariantMap& lhs, const VariantMap& rhs);
bool operator!=(const VariantMap& lhs, const VariantMap& rhs);

}  // namespace cave
