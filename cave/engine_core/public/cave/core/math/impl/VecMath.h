// =============================================================================
// File: cave/core/math/impl/VecMath.h
// =============================================================================
#pragma once
#include <cmath>
#include "cave/core/math/Scalar.h"
#include "cave/core/math/impl/Vec2.h"
#include "cave/core/math/impl/Vec3.h"
#include "cave/core/math/impl/Vec4.h"
#if USING(MATH_ENABLE_SIMD_SSE)
#include "cave/core/math/impl/VecMath.h"
#endif

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

namespace cave::math {

template<Arithmetic T, int N>
constexpr bool operator==(const Vec<T, N>& lhs, const Vec<T, N>& rhs) {
    for (int i = 0; i < N; ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

#define VECTOR_OPERATOR_VEC_VEC(DEST, OP, LHS, RHS)          \
    do {                                                     \
        constexpr int DIM = sizeof(LHS) / sizeof(LHS.x);     \
        DEST.x = LHS.x OP RHS.x;                             \
        DEST.y = LHS.y OP RHS.y;                             \
        if constexpr (DIM >= 3) { DEST.z = LHS.z OP RHS.z; } \
        if constexpr (DIM >= 4) { DEST.w = LHS.w OP RHS.w; } \
    } while (0)

#define VECTOR_OPERATOR_VEC_SCALAR(DEST, OP, LHS, RHS)     \
    do {                                                   \
        constexpr int DIM = sizeof(LHS) / sizeof(LHS.x);   \
        DEST.x = LHS.x OP RHS;                             \
        DEST.y = LHS.y OP RHS;                             \
        if constexpr (DIM >= 3) { DEST.z = LHS.z OP RHS; } \
        if constexpr (DIM >= 4) { DEST.w = LHS.w OP RHS; } \
    } while (0)

#define VECTOR_OPERATOR_SCALAR_VEC(DEST, OP, LHS, RHS)     \
    do {                                                   \
        constexpr int DIM = sizeof(RHS) / sizeof(RHS.x);   \
        DEST.x = LHS OP RHS.x;                             \
        DEST.y = LHS OP RHS.y;                             \
        if constexpr (DIM >= 3) { DEST.z = LHS OP RHS.z; } \
        if constexpr (DIM >= 4) { DEST.w = LHS OP RHS.w; } \
    } while (0)

#pragma region VECTOR_MATH_ADD
template<typename T>
FORCE_INLINE constexpr Vec<T, 2> operator+(const Vec<T, 2>& lhs, const Vec<T, 2>& rhs) {
    return { lhs.x + rhs.x, lhs.y + rhs.y };
}

template<typename T>
FORCE_INLINE constexpr Vec<T, 3> operator+(const Vec<T, 3>& lhs, const Vec<T, 3>& rhs) {
    return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}

template<typename T>
FORCE_INLINE constexpr Vec<T, 4> operator+(const Vec<T, 4>& lhs, const Vec<T, 4>& rhs) {
    return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 2> operator+(const Vec<T, 2>& lhs, const U& rhs) {
    return { lhs.x + rhs, lhs.y + rhs };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 3> operator+(const Vec<T, 3>& lhs, const U& rhs) {
    return { lhs.x + rhs, lhs.y + rhs, lhs.z + rhs };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 4> operator+(const Vec<T, 4>& lhs, const U& rhs) {
    return { lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 2> operator+(const U& lhs, const Vec<T, 2>& rhs) {
    return { lhs + rhs.x, lhs + rhs.y };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 3> operator+(const U& lhs, const Vec<T, 3>& rhs) {
    return { lhs + rhs.x, lhs + rhs.y, lhs + rhs.z };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 4> operator+(const U& lhs, const Vec<T, 4>& rhs) {
    return { lhs + rhs.x, lhs + rhs.y, lhs + rhs.z, lhs + rhs.w };
}

template<Arithmetic T>
FORCE_INLINE constexpr Vec<T, 2>& operator+=(Vec<T, 2>& lhs, const Vec<T, 2>& rhs) {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    return lhs;
}

template<Arithmetic T>
FORCE_INLINE constexpr Vec<T, 3>& operator+=(Vec<T, 3>& lhs, const Vec<T, 3>& rhs) {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    lhs.z += rhs.z;
    return lhs;
}

template<Arithmetic T>
FORCE_INLINE constexpr Vec<T, 4>& operator+=(Vec<T, 4>& lhs, const Vec<T, 4>& rhs) {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    lhs.z += rhs.z;
    lhs.w += rhs.w;
    return lhs;
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 2>& operator+=(Vec<T, 2>& lhs, const U& rhs) {
    lhs.x += rhs;
    lhs.y += rhs;
    return lhs;
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 3>& operator+=(Vec<T, 3>& lhs, const U& rhs) {
    lhs.x += rhs;
    lhs.y += rhs;
    lhs.z += rhs;
    return lhs;
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 4>& operator+=(Vec<T, 4>& lhs, const U& rhs) {
    lhs.x += rhs;
    lhs.y += rhs;
    lhs.z += rhs;
    lhs.w += rhs;
    return lhs;
}
#pragma endregion VECTOR_MATH_ADD

#pragma region VECTOR_MATH_SUB
template<typename T>
FORCE_INLINE constexpr Vec<T, 2> operator-(const Vec<T, 2>& lhs, const Vec<T, 2>& rhs) {
    return { lhs.x - rhs.x, lhs.y - rhs.y };
}

template<typename T>
FORCE_INLINE constexpr Vec<T, 3> operator-(const Vec<T, 3>& lhs, const Vec<T, 3>& rhs) {
    return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}

template<typename T>
FORCE_INLINE constexpr Vec<T, 4> operator-(const Vec<T, 4>& lhs, const Vec<T, 4>& rhs) {
    return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 2> operator-(const Vec<T, 2>& lhs, const U& rhs) {
    return { lhs.x - rhs, lhs.y - rhs };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 3> operator-(const Vec<T, 3>& lhs, const U& rhs) {
    return { lhs.x - rhs, lhs.y - rhs, lhs.z - rhs };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 4> operator-(const Vec<T, 4>& lhs, const U& rhs) {
    return { lhs.x - rhs, lhs.y - rhs, lhs.z - rhs, lhs.w - rhs };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 2> operator-(const U& lhs, const Vec<T, 2>& rhs) {
    return { lhs - rhs.x, lhs - rhs.y };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 3> operator-(const U& lhs, const Vec<T, 3>& rhs) {
    return { lhs - rhs.x, lhs - rhs.y, lhs - rhs.z };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 4> operator-(const U& lhs, const Vec<T, 4>& rhs) {
    return { lhs - rhs.x, lhs - rhs.y, lhs - rhs.z, lhs - rhs.w };
}

template<Arithmetic T>
FORCE_INLINE constexpr Vec<T, 2>& operator-=(Vec<T, 2>& lhs, const Vec<T, 2>& rhs) {
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    return lhs;
}

template<Arithmetic T>
FORCE_INLINE constexpr Vec<T, 3>& operator-=(Vec<T, 3>& lhs, const Vec<T, 3>& rhs) {
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    lhs.z -= rhs.z;
    return lhs;
}

template<Arithmetic T>
FORCE_INLINE constexpr Vec<T, 4>& operator-=(Vec<T, 4>& lhs, const Vec<T, 4>& rhs) {
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    lhs.z -= rhs.z;
    lhs.w -= rhs.w;
    return lhs;
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 2>& operator-=(Vec<T, 2>& lhs, const U& rhs) {
    lhs.x -= rhs;
    lhs.y -= rhs;
    return lhs;
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 3>& operator-=(Vec<T, 3>& lhs, const U& rhs) {
    lhs.x -= rhs;
    lhs.y -= rhs;
    lhs.z -= rhs;
    return lhs;
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 4>& operator-=(Vec<T, 4>& lhs, const U& rhs) {
    lhs.x -= rhs;
    lhs.y -= rhs;
    lhs.z -= rhs;
    lhs.w -= rhs;
    return lhs;
}
#pragma endregion VECTOR_MATH_SUB

#pragma region VECTOR_MATH_MUL
template<typename T>
FORCE_INLINE constexpr Vec<T, 2> operator*(const Vec<T, 2>& lhs, const Vec<T, 2>& rhs) {
    return { lhs.x * rhs.x, lhs.y * rhs.y };
}

template<typename T>
FORCE_INLINE constexpr Vec<T, 3> operator*(const Vec<T, 3>& lhs, const Vec<T, 3>& rhs) {
    return { lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z };
}

template<typename T>
FORCE_INLINE constexpr Vec<T, 4> operator*(const Vec<T, 4>& lhs, const Vec<T, 4>& rhs) {
    return { lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 2> operator*(const Vec<T, 2>& lhs, const U& rhs) {
    return { lhs.x * rhs, lhs.y * rhs };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 3> operator*(const Vec<T, 3>& lhs, const U& rhs) {
    return { lhs.x * rhs, lhs.y * rhs, lhs.z * rhs };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 4> operator*(const Vec<T, 4>& lhs, const U& rhs) {
    return { lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 2> operator*(const U& lhs, const Vec<T, 2>& rhs) {
    return { lhs * rhs.x, lhs * rhs.y };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 3> operator*(const U& lhs, const Vec<T, 3>& rhs) {
    return { lhs * rhs.x, lhs * rhs.y, lhs * rhs.z };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 4> operator*(const U& lhs, const Vec<T, 4>& rhs) {
    return { lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w };
}

template<Arithmetic T>
FORCE_INLINE constexpr Vec<T, 2>& operator*=(Vec<T, 2>& lhs, const Vec<T, 2>& rhs) {
    lhs.x *= rhs.x;
    lhs.y *= rhs.y;
    return lhs;
}

template<Arithmetic T>
FORCE_INLINE constexpr Vec<T, 3>& operator*=(Vec<T, 3>& lhs, const Vec<T, 3>& rhs) {
    lhs.x *= rhs.x;
    lhs.y *= rhs.y;
    lhs.z *= rhs.z;
    return lhs;
}

template<Arithmetic T>
FORCE_INLINE constexpr Vec<T, 4>& operator*=(Vec<T, 4>& lhs, const Vec<T, 4>& rhs) {
    lhs.x *= rhs.x;
    lhs.y *= rhs.y;
    lhs.z *= rhs.z;
    lhs.w *= rhs.w;
    return lhs;
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 2>& operator*=(Vec<T, 2>& lhs, const U& rhs) {
    lhs.x *= rhs;
    lhs.y *= rhs;
    return lhs;
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 3>& operator*=(Vec<T, 3>& lhs, const U& rhs) {
    lhs.x *= rhs;
    lhs.y *= rhs;
    lhs.z *= rhs;
    return lhs;
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 4>& operator*=(Vec<T, 4>& lhs, const U& rhs) {
    lhs.x *= rhs;
    lhs.y *= rhs;
    lhs.z *= rhs;
    lhs.w *= rhs;
    return lhs;
}
#pragma endregion VECTOR_MATH_MUL

#pragma region VECTOR_MATH_DIV
template<typename T>
FORCE_INLINE constexpr Vec<T, 2> operator/(const Vec<T, 2>& lhs, const Vec<T, 2>& rhs) {
    return { lhs.x / rhs.x, lhs.y / rhs.y };
}

template<typename T>
FORCE_INLINE constexpr Vec<T, 3> operator/(const Vec<T, 3>& lhs, const Vec<T, 3>& rhs) {
    return { lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z };
}

template<typename T>
FORCE_INLINE constexpr Vec<T, 4> operator/(const Vec<T, 4>& lhs, const Vec<T, 4>& rhs) {
    return { lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 2> operator/(const Vec<T, 2>& lhs, const U& rhs) {
    const U rhs_inv = static_cast<U>(1) / rhs;
    return { lhs.x * rhs_inv, lhs.y * rhs_inv };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 3> operator/(const Vec<T, 3>& lhs, const U& rhs) {
    const U rhs_inv = static_cast<U>(1) / rhs;
    return { lhs.x * rhs_inv, lhs.y * rhs_inv, lhs.z * rhs_inv };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 4> operator/(const Vec<T, 4>& lhs, const U& rhs) {
    const U rhs_inv = static_cast<U>(1) / rhs;
    return { lhs.x * rhs_inv, lhs.y * rhs_inv, lhs.z * rhs_inv, lhs.w * rhs_inv };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 2> operator/(const U& lhs, const Vec<T, 2>& rhs) {
    return { lhs / rhs.x, lhs / rhs.y };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 3> operator/(const U& lhs, const Vec<T, 3>& rhs) {
    return { lhs / rhs.x, lhs / rhs.y, lhs / rhs.z };
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 4> operator/(const U& lhs, const Vec<T, 4>& rhs) {
    return { lhs / rhs.x, lhs / rhs.y, lhs / rhs.z, lhs / rhs.w };
}

template<Arithmetic T>
FORCE_INLINE constexpr Vec<T, 2>& operator/=(Vec<T, 2>& lhs, const Vec<T, 2>& rhs) {
    lhs.x /= rhs.x;
    lhs.y /= rhs.y;
    return lhs;
}

template<Arithmetic T>
FORCE_INLINE constexpr Vec<T, 3>& operator/=(Vec<T, 3>& lhs, const Vec<T, 3>& rhs) {
    lhs.x /= rhs.x;
    lhs.y /= rhs.y;
    lhs.z /= rhs.z;
    return lhs;
}

template<Arithmetic T>
FORCE_INLINE constexpr Vec<T, 4>& operator/=(Vec<T, 4>& lhs, const Vec<T, 4>& rhs) {
    lhs.x /= rhs.x;
    lhs.y /= rhs.y;
    lhs.z /= rhs.z;
    lhs.w /= rhs.w;
    return lhs;
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 2>& operator/=(Vec<T, 2>& lhs, const U& rhs) {
    const U rhs_inv = static_cast<U>(1) / rhs;
    lhs.x *= rhs_inv;
    lhs.y *= rhs_inv;
    return lhs;
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 3>& operator/=(Vec<T, 3>& lhs, const U& rhs) {
    const U rhs_inv = static_cast<U>(1) / rhs;
    lhs.x *= rhs_inv;
    lhs.y *= rhs_inv;
    lhs.z *= rhs_inv;
    return lhs;
}

template<Arithmetic T, Arithmetic U>
FORCE_INLINE constexpr Vec<T, 4>& operator/=(Vec<T, 4>& lhs, const U& rhs) {
    const U rhs_inv = static_cast<U>(1) / rhs;
    lhs.x *= rhs_inv;
    lhs.y *= rhs_inv;
    lhs.z *= rhs_inv;
    lhs.w *= rhs_inv;
    return lhs;
}
#pragma endregion VECTOR_MATH_DIV

#pragma region VECTOR_NEGATION
template<typename T>
FORCE_INLINE constexpr Vec<T, 2> operator-(const Vec<T, 2>& v) {
    return { -v.x, -v.y };
}

template<typename T>
FORCE_INLINE constexpr Vec<T, 3> operator-(const Vec<T, 3>& v) {
    return { -v.x, -v.y, -v.z };
}

template<typename T>
FORCE_INLINE constexpr Vec<T, 4> operator-(const Vec<T, 4>& v) {
    return { -v.x, -v.y, -v.z, -v.w };
}
#pragma endregion VECTOR_NEGATION

template<Arithmetic T, int N>
constexpr inline Vec<T, N> min(const Vec<T, N>& lhs, const Vec<T, N>& rhs) {
    Vec<T, N> result;
    result.x = min(lhs.x, rhs.x);
    result.y = min(lhs.y, rhs.y);
    if constexpr (N >= 3) {
        result.z = min(lhs.z, rhs.z);
    }
    if constexpr (N >= 4) {
        result.w = min(lhs.w, rhs.w);
    }
    return result;
}

template<Arithmetic T, int N>
constexpr inline Vec<T, N> max(const Vec<T, N>& lhs, const Vec<T, N>& rhs) {
    Vec<T, N> result;
    result.x = max(lhs.x, rhs.x);
    result.y = max(lhs.y, rhs.y);
    if constexpr (N >= 3) {
        result.z = max(lhs.z, rhs.z);
    }
    if constexpr (N >= 4) {
        result.w = max(lhs.w, rhs.w);
    }
    return result;
}

template<Arithmetic T, int N>
constexpr inline Vec<T, N> abs(const Vec<T, N>& lhs) {
    Vec<T, N> result;
    result.x = abs(lhs.x);
    result.y = abs(lhs.y);
    if constexpr (N >= 3) {
        result.z = abs(lhs.z);
    }
    if constexpr (N >= 4) {
        result.w = abs(lhs.w);
    }
    return result;
}

template<Arithmetic T, int N>
constexpr inline Vec<T, N> clamp(const Vec<T, N>& value, const Vec<T, N>& low, const Vec<T, N>& high) {
    return max(low, min(value, high));
}

template<FloatingPoint T, int N>
constexpr inline Vec<T, N> lerp(const Vec<T, N>& x, const Vec<T, N>& y, float s) {
    return (static_cast<T>(1) - s) * x + s * y;
}

template<Arithmetic T, int N>
constexpr inline T dot(const Vec<T, N>& lhs, const Vec<T, N>& rhs) {
    Vec<T, N> tmp(lhs * rhs);
    T result = tmp.x + tmp.y;
    if constexpr (N >= 3) {
        result += tmp.z;
    }
    if constexpr (N >= 4) {
        result += tmp.w;
    }
    return result;
}

template<Arithmetic T, int N>
    requires(std::is_floating_point_v<T>)
constexpr inline T length(const Vec<T, N>& lhs) {
    return std::sqrt(dot(lhs, lhs));
}

template<Arithmetic T, int N>
    requires(std::is_floating_point_v<T>)
constexpr inline Vec<T, N> normalize(const Vec<T, N>& lhs) {
    const auto inverse_length = static_cast<T>(1) / length(lhs);
    return lhs * inverse_length;
}

template<Arithmetic T>
FORCE_INLINE constexpr Vec<T, 3> cross(const Vec<T, 3>& lhs, const Vec<T, 3>& rhs) {
    return {
        lhs.y * rhs.z - rhs.y * lhs.z,
        lhs.z * rhs.x - rhs.z * lhs.x,
        lhs.x * rhs.y - rhs.x * lhs.y
    };
}

}  // namespace cave::math
