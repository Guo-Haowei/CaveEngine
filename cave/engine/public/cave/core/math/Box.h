#pragma once
#include "cave/core/math/Vector.h"

namespace cave::math {

template<typename T, int N>
class Box {
    static_assert(N == 2 || N == 3);
    static_assert(std::is_floating_point_v<T>);

    using Scalar = T;
    using Vec = Vector<Scalar, N>;
    using Self = Box<Scalar, N>;

    // Invariant (half-open):
    // - Valid: min[i] <= max[i] for all i
    // - Empty: any min[i] == max[i]
    // - Invalid: min/max are NaN or explicitly invalidated (we use +inf/-inf sentinel)
public:
    static constexpr Scalar BOX_MIN_SIZE = Scalar(0.0001);

    constexpr Box() noexcept { invalidate(); }

    constexpr Box(const Vec& min, const Vec& max) noexcept
        : min_(min), max_(max) {}

    static constexpr Self fromCenterHalfExtent(const Vec& center, const Vec& half) noexcept {
        return Self(center - half, center + half);
    }

    constexpr const Vec& min() const noexcept { return min_; }
    constexpr const Vec& max() const noexcept { return max_; }
    constexpr void setMinMax(const Vec& min, const Vec& max) noexcept {
        min_ = min;
        max_ = max;
    }

    constexpr Vec center() const noexcept { return (min_ + max_) * Scalar(0.5); }
    constexpr Vec size() const noexcept { return max_ - min_; }
    constexpr Vec halfExtent() const noexcept { return (max_ - min_) * Scalar(0.5); }

    void invalidate() {
        constexpr Scalar inf = std::numeric_limits<Scalar>::infinity();
        min_ = Vec(inf);
        max_ = Vec(-inf);
    }

    constexpr bool contains(const Vec& point) const noexcept {
        if (!isValid()) return false;
        if (point.x < min_.x || point.x > max_.x) return false;
        if (point.y < min_.y || point.y > max_.y) return false;
        if constexpr (N == 3)
            if (point.z < min_.z || point.z > max_.z) return false;
        return true;
    }

    bool isValid() const {
        if (min_.x >= max_.x) return false;
        if (min_.y >= max_.y) return false;
        if constexpr (N == 3)
            if (min_.z >= max_.z) return false;
        return true;
    }

    void makeValid() {
        const Vec size = max_ - min_;
        if (size.x == 0.0f) {
            min_.x -= BOX_MIN_SIZE;
            max_.x += BOX_MIN_SIZE;
        }
        if (size.y == 0.0f) {
            min_.y -= BOX_MIN_SIZE;
            max_.y += BOX_MIN_SIZE;
        }
        if constexpr (N > 2) {
            if (size.z == 0.0f) {
                min_.z -= BOX_MIN_SIZE;
                max_.z += BOX_MIN_SIZE;
            }
        }
    }

    void expandToInclude(const Vec& point) {
        min_ = math::min(min_, point);
        max_ = math::max(max_, point);
    }

    void expandToInclude(const Self& box) {
        min_ = math::min(min_, box.min_);
        max_ = math::max(max_, box.max_);
    }

    void clip(const Self& box) {
        min_ = math::max(min_, box.min_);
        max_ = math::min(max_, box.max_);
    }

    constexpr bool intersects(const Self& other) const noexcept {
        if (!isValid() || !other.isValid()) return false;
        if (max_.x <= other.min_.x || min_.x >= other.max_.x) return false;
        if (max_.y <= other.min_.y || min_.y >= other.max_.y) return false;
        if constexpr (N == 3) {
            if (max_.z <= other.min_.z || min_.z >= other.max_.z) return false;
        }
        return true;
    }

    constexpr bool touchesOrIntersects(const Self& other) const noexcept {
        if (!isValid() || !other.isValid()) return false;
        if (max_.x < other.min_.x || min_.x > other.max_.x) return false;
        if (max_.y < other.min_.y || min_.y > other.max_.y) return false;
        if constexpr (N == 3) {
            if (max_.z < other.min_.z || min_.z > other.max_.z) return false;
        }
        return true;
    }

protected:
    Vec min_;
    Vec max_;
};

using Box2 = Box<float, 2>;
using Box3 = Box<float, 3>;

}  // namespace cave::math
