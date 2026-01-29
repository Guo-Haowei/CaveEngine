#pragma once
#include "Vector.h"

// @TODO: use new implementation
#if 0
template<int N, typename T>
class Box {
    static_assert(N > 0);
    static_assert(std::is_floating_point_v<T>);

public:
    using Scalar = T;
    using Vec    = Vector<T, N>;
    using Self   = Box<N, T>;

    // Invariant (half-open):
    // - Valid: min[i] <= max[i] for all i
    // - Empty: any min[i] == max[i]
    // - Invalid: min/max are NaN or explicitly invalidated (we use +inf/-inf sentinel)

    constexpr Box() noexcept { Invalidate(); }
    constexpr Box(const Vec& min, const Vec& max) noexcept : m_min(min), m_max(max) {}

    // Factories
    static constexpr Self FromMinMax(const Vec& min, const Vec& max) noexcept { return Self(min, max); }
    static constexpr Self FromCenterHalfExtent(const Vec& c, const Vec& he) noexcept { return Self(c - he, c + he); }

    // State
    constexpr void Invalidate() noexcept {
        const T inf = std::numeric_limits<T>::infinity();
        m_min = Vec(inf);
        m_max = Vec(-inf);
    }

    [[nodiscard]] constexpr bool IsValid() const noexcept {
        for (int i = 0; i < N; ++i) {
            if (!(m_min[i] <= m_max[i])) return false;
        }
        return true;
    }

    [[nodiscard]] constexpr bool IsEmpty() const noexcept {
        if (!IsValid()) return true;
        for (int i = 0; i < N; ++i) {
            if (m_min[i] == m_max[i]) return true;
        }
        return false;
    }

    // Access
    [[nodiscard]] constexpr const Vec& Min() const noexcept { return m_min; }
    [[nodiscard]] constexpr const Vec& Max() const noexcept { return m_max; }
    constexpr void SetMinMax(const Vec& min, const Vec& max) noexcept { m_min = min; m_max = max; }

    // Geometry
    [[nodiscard]] constexpr Vec Center() const noexcept { return (m_min + m_max) * T(0.5); }
    [[nodiscard]] constexpr Vec Size() const noexcept { return m_max - m_min; }
    [[nodiscard]] constexpr Vec HalfExtent() const noexcept { return (m_max - m_min) * T(0.5); }

    // Expand/Combine
    constexpr void Expand(const Vec& p) noexcept {
        if (!IsValid()) { m_min = p; m_max = p; return; }
        for (int i = 0; i < N; ++i) {
            if (p[i] < m_min[i]) m_min[i] = p[i];
            if (p[i] > m_max[i]) m_max[i] = p[i];
        }
    }

    constexpr void Expand(const Self& b) noexcept {
        if (!b.IsValid()) return;
        if (!IsValid()) { *this = b; return; }
        for (int i = 0; i < N; ++i) {
            if (b.m_min[i] < m_min[i]) m_min[i] = b.m_min[i];
            if (b.m_max[i] > m_max[i]) m_max[i] = b.m_max[i];
        }
    }

    [[nodiscard]] static constexpr Self Union(Self a, const Self& b) noexcept { a.Expand(b); return a; }

    constexpr void Intersect(const Self& b) noexcept {
        if (!IsValid() || !b.IsValid()) { Invalidate(); return; }
        for (int i = 0; i < N; ++i) {
            if (b.m_min[i] > m_min[i]) m_min[i] = b.m_min[i];
            if (b.m_max[i] < m_max[i]) m_max[i] = b.m_max[i];
        }
        if (!IsValid()) Invalidate(); // disjoint => invalid
    }

    [[nodiscard]] static constexpr Self Intersection(Self a, const Self& b) noexcept { a.Intersect(b); return a; }

    // Queries
    [[nodiscard]] constexpr bool Contains(const Vec& p) const noexcept {
        if (!IsValid()) return false;
        for (int i = 0; i < N; ++i) {
            if (p[i] < m_min[i] || p[i] > m_max[i]) return false;
        }
        return true;
    }

    [[nodiscard]] constexpr bool Overlaps(const Self& b) const noexcept {
        if (!IsValid() || !b.IsValid()) return false;
        for (int i = 0; i < N; ++i) {
            if (m_max[i] < b.m_min[i] || b.m_max[i] < m_min[i]) return false;
        }
        return true;
    }

    // Utility: clamp / closest point
    [[nodiscard]] constexpr Vec ClampPoint(const Vec& p) const noexcept {
        Vec out = p;
        if (!IsValid()) return out;
        for (int i = 0; i < N; ++i) {
            if (out[i] < m_min[i]) out[i] = m_min[i];
            if (out[i] > m_max[i]) out[i] = m_max[i];
        }
        return out;
    }

    // Transform-lite
    constexpr void Translate(const Vec& d) noexcept {
        if (!IsValid()) return;
        m_min += d;
        m_max += d;
    }

    [[nodiscard]] constexpr Self Translated(const Vec& d) const noexcept {
        Self out = *this;
        out.Translate(d);
        return out;
    }

private:
    Vec m_min{};
    Vec m_max{};
};

template<int N, typename T>
constexpr void FixDegenerate(Box<N, T>& b, T min_extent) noexcept {
    if (!b.IsValid()) return;
    auto minv = b.Min();
    auto maxv = b.Max();

    for (int i = 0; i < N; ++i) {
        const T e = maxv[i] - minv[i];
        if (e < min_extent) {
            const T c = (minv[i] + maxv[i]) * T(0.5);
            const T h = min_extent * T(0.5);
            minv[i] = c - h;
            maxv[i] = c + h;
        }
    }
    b.SetMinMax(minv, maxv);
}

#endif

namespace cave::math {

template<int N>
class Box {
    using VecT = Vector<float, N>;
    using Self = Box<N>;

public:
    static constexpr float BOX_MIN_SIZE = 0.0001f;

    Box() { MakeInvalid(); }

    Box(const VecT& p_min, const VecT& p_max)
        : m_min(p_min), m_max(p_max) {}

    void MakeInvalid() {
        m_min = VecT(std::numeric_limits<float>::infinity());
        m_max = VecT(-std::numeric_limits<float>::infinity());
    }

    bool IsValid() const {
        for (int i = 0; i < N; ++i) {
            if (m_min[i] >= m_max[i]) {
                return false;
            }
        }
        return true;
    }

    void MakeValid() {
        const VecT size = m_max - m_min;
        if (size.x == 0.0f) {
            m_min.x -= BOX_MIN_SIZE;
            m_max.x += BOX_MIN_SIZE;
        }
        if (size.y == 0.0f) {
            m_min.y -= BOX_MIN_SIZE;
            m_max.y += BOX_MIN_SIZE;
        }
        if constexpr (N > 2) {
            if (size.z == 0.0f) {
                m_min.z -= BOX_MIN_SIZE;
                m_max.z += BOX_MIN_SIZE;
            }
        }
    }

    void ExpandPoint(const VecT& p_point) {
        m_min = min(m_min, p_point);
        m_max = max(m_max, p_point);
    }

    void UnionBox(const Self& p_other) {
        m_min = min(m_min, p_other.m_min);
        m_max = max(m_max, p_other.m_max);
    }

    void IntersectBox(const Self& p_other) {
        m_min = max(m_min, p_other.m_min);
        m_max = min(m_max, p_other.m_max);
    }

    float SurfaceArea() const;

    VecT Center() const { return 0.5f * (m_min + m_max); }
    VecT Size() const { return m_max - m_min; }

    const VecT& GetMin() const { return m_min; }
    const VecT& GetMax() const { return m_max; }

protected:
    VecT m_min;
    VecT m_max;
};

using Box2 = Box<2>;
using Box3 = Box<3>;

}  // namespace cave::math
