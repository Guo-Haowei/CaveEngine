// =============================================================================
// File: public/cave/core/math/Rect.h
// =============================================================================
#pragma once
#include <concepts>
#include "cave/core/math/impl/Vector2.h"

namespace cave::math {

template<typename T>
    requires(std::integral<T> || std::floating_point<T>)
struct Rect {
    T x{};
    T y{};
    T w{};
    T h{};

    inline constexpr T Left() const { return x; }
    inline constexpr T Right() const { return x + w; }
    inline constexpr T Top() const { return y; }
    inline constexpr T Bottom() const { return y + h; }
    inline constexpr T Width() const { return w; }
    inline constexpr T Height() const { return h; }

    static Rect<T> FromMinMax(T p_min_x, T p_min_y, T p_max_x, T p_max_y) {
        return { p_min_x, p_min_y, p_max_x - p_min_x, p_max_y - p_min_y };
    }

    inline constexpr bool Contains(T p_x, T p_y) const {
        return p_x >= Left() && p_x < Right() && p_y >= Top() && p_y < Bottom();
    }

    constexpr Vector<T, 2> Min() const {
        return { x, y };
    }

    constexpr Vector<T, 2> Max() const {
        return { x + w, y + h };
    }

    constexpr Vector<T, 2> Extent() const {
        return { w, h };
    }
};

using IntRect = Rect<int>;
using FloatRect = Rect<float>;

}  // namespace cave::math
