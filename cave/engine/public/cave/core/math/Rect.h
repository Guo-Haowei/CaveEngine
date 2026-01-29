// =============================================================================
// File: public/cave/core/math/Rect.h
// =============================================================================
#pragma once
#include <concepts>

namespace cave::math {

template<typename T>
    requires(std::integral<T> || std::floating_point<T>)
struct Rect {
    T x{};
    T y{};
    T w{};
    T h{};

    T Left() const { return x; }
    T Right() const { return x + w; }
    T Top() const { return y; }
    T Bottom() const { return y + h; }

    static Rect<T> FromMinMax(T min_x, T min_y, T max_x, T max_y) {
        return { min_x, min_y, max_x - min_x, max_y - min_y };
    }
};

using IntRect = Rect<int>;
using FloatRect = Rect<float>;

}  // namespace cave::math
