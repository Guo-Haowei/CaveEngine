#pragma once
#include "cave/core/math/Vector.h"

namespace cave::math {

class Plane {
public:
    constexpr Plane() = default;

    constexpr Plane(const Vec3f& normal, float constant) noexcept
        : normal_(normal), constant_(constant) {}

    float distance(const Vec3f& point) const {
        return dot(point, normal_) + constant_;
    }

    const Vec3f& normal() const { return normal_; }
    float constant() const { return constant_; }

    static constexpr Plane xy() {
        return { Vec3f::UnitZ, 0.0f };
    };

    static constexpr Plane xz() {
        return { Vec3f::UnitY, 0.0f };
    };

    static constexpr Plane yz() {
        return { Vec3f::UnitX, 0.0f };
    };

private:
    Vec3f normal_;
    float constant_;
};

}  // namespace cave::math
