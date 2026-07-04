#include "cave/core/variant/Variant.h"

namespace cave {

using namespace ::cave::math;

Variant::Variant(bool value) : type_(VariantType::Int), int_(value ? 1 : 0) {}

Variant::Variant(int value) : type_(VariantType::Int), int_(value) {}

Variant::Variant(float value) : type_(VariantType::Float), float_(value) {}

Variant::Variant(const char* value) : type_(VariantType::String), int_(0), string_(value) {}

Variant::Variant(std::string_view value) : type_(VariantType::String), int_(0), string_(value) {}

Variant::Variant(std::string value) : type_(VariantType::String), int_(0), string_(std::move(value)) {}

Variant::Variant(const Vec2f& value) : type_(VariantType::Vec2f), vec_(Vec4f(value, 0.0f, 0.0f)) {}

Variant::Variant(const Vec3f& value) : type_(VariantType::Vec3f), vec_(Vec4f(value, 0.0f)) {}

Variant::Variant(const Vec4f& value) : type_(VariantType::Vec4f), vec_(value) {}

Variant::Variant(const Vec2i& value) : type_(VariantType::Vec2i), ivec_(Vec4i(value, 0, 0)) {}

Variant::Variant(const Vec3i& value) : type_(VariantType::Vec3i), ivec_(Vec4i(value, 0)) {}

Variant::Variant(const Vec4i& value) : type_(VariantType::Vec4i), ivec_(value) {}

bool Variant::isNumeric() const {
    return type_ == VariantType::Int || type_ == VariantType::Float;
}

bool Variant::asBool(bool fallback) const {
    if (type_ == VariantType::Int) {
        return int_ != 0;
    }
    if (type_ == VariantType::Float) {
        return float_ != 0.0f;
    }
    return fallback;
}

int Variant::asInt(int fallback) const {
    if (type_ == VariantType::Int) {
        return int_;
    }
    if (type_ == VariantType::Float) {
        return static_cast<int>(float_);
    }
    return fallback;
}

float Variant::asFloat(float fallback) const {
    if (type_ == VariantType::Float) {
        return float_;
    }
    if (type_ == VariantType::Int) {
        return static_cast<float>(int_);
    }
    return fallback;
}

std::string_view Variant::asString(std::string_view fallback) const {
    return type_ == VariantType::String ? std::string_view(string_) : fallback;
}

Vec2f Variant::asVec2f(Vec2f fallback) const {
    return type_ == VariantType::Vec2f ? vec_.xy : fallback;
}

Vec3f Variant::asVec3f(Vec3f fallback) const {
    return type_ == VariantType::Vec3f ? vec_.xyz : fallback;
}

Vec4f Variant::asVec4f(Vec4f fallback) const {
    return type_ == VariantType::Vec4f ? vec_ : fallback;
}

Vec2i Variant::asVec2i(Vec2i fallback) const {
    return type_ == VariantType::Vec2i ? ivec_.xy : fallback;
}

Vec3i Variant::asVec3i(Vec3i fallback) const {
    return type_ == VariantType::Vec3i ? ivec_.xyz : fallback;
}

Vec4i Variant::asVec4i(Vec4i fallback) const {
    return type_ == VariantType::Vec4i ? ivec_ : fallback;
}

}  // namespace cave
