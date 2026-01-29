#pragma once
#include "cave/core/math/Vector.h"

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
using OldRect = Box2;

}  // namespace cave::math
