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
    static constexpr Scalar kBoxMinSize = Scalar(0.0001);

    constexpr Box() noexcept { invalidate(); }

    constexpr Box(const Vec& min, const Vec& max) noexcept
        : m_min(min), m_max(max) {}

    static constexpr Self fromCenterHalfExtent(const Vec& center, const Vec& half) noexcept {
        return Self(center - half, center + half);
    }

    constexpr const Vec& min() const noexcept { return m_min; }
    constexpr const Vec& max() const noexcept { return m_max; }
    constexpr void setMinMax(const Vec& min, const Vec& max) noexcept {
        m_min = min;
        m_max = max;
    }

    constexpr Vec center() const noexcept { return (m_min + m_max) * Scalar(0.5); }
    constexpr Vec size() const noexcept { return m_max - m_min; }
    constexpr Vec halfExtent() const noexcept { return (m_max - m_min) * Scalar(0.5); }

    void invalidate() {
        constexpr Scalar inf = std::numeric_limits<Scalar>::infinity();
        m_min = Vec(inf);
        m_max = Vec(-inf);
    }

    constexpr bool contains(const Vec& point) const noexcept {
        if (!valid()) return false;
        if (point.x < m_min.x || point.x > m_max.x) return false;
        if (point.y < m_min.y || point.y > m_max.y) return false;
        if constexpr (N == 3)
            if (point.z < m_min.z || point.z > m_max.z) return false;
        return true;
    }

    bool valid() const {
        if (m_min.x >= m_max.x) return false;
        if (m_min.y >= m_max.y) return false;
        if constexpr (N == 3)
            if (m_min.z >= m_max.z) return false;
        return true;
    }

    void expandIfDegenerate() {
        const Vec size = m_max - m_min;
        if (size.x == 0.0f) {
            m_min.x -= kBoxMinSize;
            m_max.x += kBoxMinSize;
        }
        if (size.y == 0.0f) {
            m_min.y -= kBoxMinSize;
            m_max.y += kBoxMinSize;
        }
        if constexpr (N > 2) {
            if (size.z == 0.0f) {
                m_min.z -= kBoxMinSize;
                m_max.z += kBoxMinSize;
            }
        }
    }

    void expandToInclude(const Vec& point) {
        m_min = math::min(m_min, point);
        m_max = math::max(m_max, point);
    }

    void expandToInclude(const Self& box) {
        m_min = math::min(m_min, box.m_min);
        m_max = math::max(m_max, box.m_max);
    }

    void clip(const Self& box) {
        m_min = math::max(m_min, box.m_min);
        m_max = math::min(m_max, box.m_max);
    }

    constexpr bool intersects(const Self& other) const noexcept {
        if (!valid() || !other.valid()) return false;
        if (m_max.x <= other.m_min.x || m_min.x >= other.m_max.x) return false;
        if (m_max.y <= other.m_min.y || m_min.y >= other.m_max.y) return false;
        if constexpr (N == 3) {
            if (m_max.z <= other.m_min.z || m_min.z >= other.m_max.z) return false;
        }
        return true;
    }

    constexpr bool touchesOrIntersects(const Self& other) const noexcept {
        if (!valid() || !other.valid()) return false;
        if (m_max.x < other.m_min.x || m_min.x > other.m_max.x) return false;
        if (m_max.y < other.m_min.y || m_min.y > other.m_max.y) return false;
        if constexpr (N == 3) {
            if (m_max.z < other.m_min.z || m_min.z > other.m_max.z) return false;
        }
        return true;
    }

protected:
    Vec m_min;
    Vec m_max;
};

using Box2 = Box<float, 2>;
using Box3 = Box<float, 3>;

}  // namespace cave::math
