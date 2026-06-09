// =============================================================================
// File: cave/core/math/Scalar.h
// =============================================================================
#pragma once
#include <concepts>
#include <cstdlib>
#include <limits>

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

namespace cave::math {

template<typename T = float>
    requires std::is_floating_point_v<T>
constexpr inline T pi() {
    return static_cast<T>(3.14159265358979323846264338327950288);
}

template<typename T = float>
    requires std::is_floating_point_v<T>
constexpr inline T halfPi() {
    return static_cast<T>(0.5) * pi();
}

template<typename T = float>
    requires std::is_floating_point_v<T>
constexpr inline T twoPi() {
    return static_cast<T>(2) * pi();
}

template<typename T = float>
    requires std::is_floating_point_v<T>
constexpr inline T epsilon() {
    return std::numeric_limits<T>::epsilon();
}

template<typename T>
    requires std::is_floating_point_v<T>
constexpr inline T radians(const T& p_degrees) {
    return p_degrees * static_cast<T>(0.01745329251994329576923690768489);
}

template<typename T>
    requires std::is_floating_point_v<T>
constexpr inline T degrees(const T& p_radians) {
    return p_radians * static_cast<T>(57.295779513082320876798154814105);
}

// for glsl-like usage
template<typename T>
constexpr inline T min(const T& a, const T& b) {
    return a < b ? a : b;
}

template<typename T>
constexpr inline T max(const T& a, const T& b) {
    return a > b ? a : b;
}

template<typename T>
constexpr inline T abs(const T& v) {
    return std::abs(v);
}

template<typename T>
constexpr inline T clamp(const T& v, const T& low, const T& high) {
    return math::max(low, math::min(v, high));
}

}  // namespace cave::math
