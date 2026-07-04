#include "cave/core/variant/Variant.h"

#include <format>

namespace cave {

using namespace ::cave::math;

Variant::Variant(bool value)
    : type_(VariantType::Int), int_(value ? 1 : 0) {}

Variant::Variant(int value)
    : type_(VariantType::Int), int_(value) {}

Variant::Variant(float value)
    : type_(VariantType::Float), float_(value) {}

Variant::Variant(const char* value)
    : type_(VariantType::String), int_(0), string_(value) {}

Variant::Variant(std::string_view value)
    : type_(VariantType::String), int_(0), string_(value) {}

Variant::Variant(std::string value)
    : type_(VariantType::String), int_(0), string_(std::move(value)) {}

Variant::Variant(const Vec2f& value)
    : type_(VariantType::Vec2f), vec_(Vec4f(value, 0.0f, 0.0f)) {}

Variant::Variant(const Vec3f& value)
    : type_(VariantType::Vec3f), vec_(Vec4f(value, 0.0f)) {}

Variant::Variant(const Vec4f& value)
    : type_(VariantType::Vec4f), vec_(value) {}

Variant::Variant(const Vec2i& value)
    : type_(VariantType::Vec2i), ivec_(Vec4i(value, 0, 0)) {}

Variant::Variant(const Vec3i& value)
    : type_(VariantType::Vec3i), ivec_(Vec4i(value, 0)) {}

Variant::Variant(const Vec4i& value)
    : type_(VariantType::Vec4i), ivec_(value) {}

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

void* Variant::asPointer() {
    switch (type_) {
        case VariantType::Int:
        case VariantType::Float:
        case VariantType::Vec2f:
        case VariantType::Vec3f:
        case VariantType::Vec4f:
        case VariantType::Vec2i:
        case VariantType::Vec3i:
        case VariantType::Vec4i:
            return &int_;
        default:
            CRASH_NOW();
            return nullptr;
    }
}

bool Variant::operator==(const Variant& rhs) const {
    if (type_ != rhs.type_) {
        return false;
    }

    switch (type_) {
        case VariantType::Invalid:
            return true;
        case VariantType::Int:
            return int_ == rhs.int_;
        case VariantType::Float:
            return float_ == rhs.float_;
        case VariantType::String:
            return string_ == rhs.string_;
        case VariantType::Vec2f:
            return asVec2f() == rhs.asVec2f();
        case VariantType::Vec3f:
            return asVec3f() == rhs.asVec3f();
        case VariantType::Vec4f:
            return vec_ == rhs.vec_;
        case VariantType::Vec2i:
            return asVec2i() == rhs.asVec2i();
        case VariantType::Vec3i:
            return asVec3i() == rhs.asVec3i();
        case VariantType::Vec4i:
            return ivec_ == rhs.ivec_;
        default:
            return false;
    }
}

// @TODO: serializer?
std::string Variant::toString() const {
    switch (type()) {
        case VariantType::Int:
            return std::format("{}", int_);
        case VariantType::Float:
            return std::format("{}", float_);
        case VariantType::String:
            return std::format("\"{}\"", string_);
        case VariantType::Vec2f:
            return std::format("{} {}", vec_.x, vec_.y);
        case VariantType::Vec2i:
            return std::format("{} {}", ivec_.x, ivec_.y);
        case VariantType::Vec3f:
            return std::format("{} {} {}", vec_.x, vec_.y, vec_.z);
        case VariantType::Vec3i:
            return std::format("{} {} {}", ivec_.x, ivec_.y, ivec_.z);
        case VariantType::Vec4f:
            return std::format("{} {} {} {}", vec_.x, vec_.y, vec_.z, vec_.w);
        case VariantType::Vec4i:
            return std::format("{} {} {} {}", ivec_.x, ivec_.y, ivec_.z, ivec_.w);
        default:
            CRASH_NOW();
            return std::string{};
    }
}

Variant Variant::makeDefault(VariantType type) {
    switch (type) {
        case VariantType::Int:
            return Variant(0);
        case VariantType::Float:
            return Variant(0.0f);
        case VariantType::String:
            return Variant("");
        case VariantType::Vec2f:
            return Variant(math::Vec2f::Zero);
        case VariantType::Vec3f:
            return Variant(math::Vec3f::Zero);
        case VariantType::Vec4f:
            return Variant(math::Vec4f::Zero);
        case VariantType::Vec2i:
            return Variant(math::Vec2i::Zero);
        case VariantType::Vec3i:
            return Variant(math::Vec3i::Zero);
        case VariantType::Vec4i:
            return Variant(math::Vec4i::Zero);
        default:
            return Variant();
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
