#pragma once
#include "Vector.h"

namespace cave::math {

class Plane {
public:
    constexpr Plane() = default;

    constexpr Plane(const Vector3f& normal, float constant) noexcept
        : normal_(normal), constant_(constant) {}

    float distance(const Vector3f& point) const {
        return dot(point, normal_) + constant_;
    }

    const Vector3f& normal() const { return normal_; }
    float constant() const { return constant_; }

    static constexpr Plane xy() {
        return { Vector3f::UnitZ, 0.0f };
    };

    static constexpr Plane xz() {
        return { Vector3f::UnitY, 0.0f };
    };

    static constexpr Plane yz() {
        return { Vector3f::UnitX, 0.0f };
    };

private:
    Vector3f normal_;
    float constant_;
};

}  // namespace cave::math
