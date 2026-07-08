// =============================================================================
// File: cave/core/math/impl/VecMath.h
// =============================================================================
#pragma once
#include <xmmintrin.h>

#include "cave/core/math/Scalar.h"
#include "cave/core/math/impl/Vec2.h"
#include "cave/core/math/impl/Vec3.h"
#include "cave/core/math/impl/Vec4.h"

namespace cave::math {

static_assert(alignof(__m128) == 16);

FORCE_INLINE __m128 vector_add_sse(__m128 lhs, __m128 rhs) {
    return _mm_add_ps(lhs, rhs);
}

FORCE_INLINE __m128 vector_sub_sse(__m128 lhs, __m128 rhs) {
    return _mm_sub_ps(lhs, rhs);
}

FORCE_INLINE __m128 vector_mul_sse(__m128 lhs, __m128 rhs) {
    return _mm_mul_ps(lhs, rhs);
}

FORCE_INLINE __m128 vector_div_sse(__m128 lhs, __m128 rhs) {
    return _mm_div_ps(lhs, rhs);
}

// ---- Addition ----

FORCE_INLINE Vec<float, 4> operator+(const Vec<float, 4>& lhs, const Vec<float, 4>& rhs) {
    return vector_add_sse(lhs.simd, rhs.simd);
}

template<Arithmetic U>
FORCE_INLINE Vec<float, 4> operator+(const Vec<float, 4>& lhs, const U& rhs) {
    __m128 scalar = _mm_set1_ps(rhs);
    return vector_add_sse(lhs.simd, scalar);
}

template<Arithmetic U>
FORCE_INLINE Vec<float, 4> operator+(const U& lhs, const Vec<float, 4>& rhs) {
    __m128 scalar = _mm_set1_ps(lhs);
    return vector_add_sse(scalar, rhs.simd);
}

FORCE_INLINE Vec<float, 4>& operator+=(Vec<float, 4>& lhs, const Vec<float, 4>& rhs) {
    lhs.simd = vector_add_sse(lhs.simd, rhs.simd);
    return lhs;
}

template<Arithmetic U>
FORCE_INLINE Vec<float, 4>& operator+=(Vec<float, 4>& lhs, const U& rhs) {
    __m128 scalar = _mm_set1_ps(rhs);
    lhs.simd = vector_add_sse(lhs.simd, scalar);
    return lhs;
}

// ---- Subtraction ----

FORCE_INLINE Vec<float, 4> operator-(const Vec<float, 4>& lhs, const Vec<float, 4>& rhs) {
    return vector_sub_sse(lhs.simd, rhs.simd);
}

template<Arithmetic U>
FORCE_INLINE Vec<float, 4> operator-(const Vec<float, 4>& lhs, const U& rhs) {
    __m128 scalar = _mm_set1_ps(rhs);
    return vector_sub_sse(lhs.simd, scalar);
}

template<Arithmetic U>
FORCE_INLINE Vec<float, 4> operator-(const U& lhs, const Vec<float, 4>& rhs) {
    __m128 scalar = _mm_set1_ps(lhs);
    return vector_sub_sse(scalar, rhs.simd);
}

FORCE_INLINE Vec<float, 4>& operator-=(Vec<float, 4>& lhs, const Vec<float, 4>& rhs) {
    lhs.simd = vector_sub_sse(lhs.simd, rhs.simd);
    return lhs;
}

template<Arithmetic U>
FORCE_INLINE Vec<float, 4>& operator-=(Vec<float, 4>& lhs, const U& rhs) {
    __m128 scalar = _mm_set1_ps(rhs);
    lhs.simd = vector_sub_sse(lhs.simd, scalar);
    return lhs;
}

// ---- Multiplication ----

FORCE_INLINE Vec<float, 4> operator*(const Vec<float, 4>& lhs, const Vec<float, 4>& rhs) {
    return vector_mul_sse(lhs.simd, rhs.simd);
}

template<Arithmetic U>
FORCE_INLINE Vec<float, 4> operator*(const Vec<float, 4>& lhs, const U& rhs) {
    __m128 scalar = _mm_set1_ps(rhs);
    return vector_mul_sse(lhs.simd, scalar);
}

template<Arithmetic U>
FORCE_INLINE Vec<float, 4> operator*(const U& lhs, const Vec<float, 4>& rhs) {
    __m128 scalar = _mm_set1_ps(lhs);
    return vector_mul_sse(scalar, rhs.simd);
}

FORCE_INLINE Vec<float, 4>& operator*=(Vec<float, 4>& lhs, const Vec<float, 4>& rhs) {
    lhs.simd = vector_mul_sse(lhs.simd, rhs.simd);
    return lhs;
}

template<Arithmetic U>
FORCE_INLINE Vec<float, 4>& operator*=(Vec<float, 4>& lhs, const U& rhs) {
    __m128 scalar = _mm_set1_ps(rhs);
    lhs.simd = vector_mul_sse(lhs.simd, scalar);
    return lhs;
}

// ---- DIVISION ----

FORCE_INLINE Vec<float, 4> operator/(const Vec<float, 4>& lhs, const Vec<float, 4>& rhs) {
    return vector_div_sse(lhs.simd, rhs.simd);
}

template<Arithmetic U>
FORCE_INLINE Vec<float, 4> operator/(const Vec<float, 4>& lhs, const U& rhs) {
    __m128 scalar = _mm_set1_ps(rhs);
    return vector_div_sse(lhs.simd, scalar);
}

template<Arithmetic U>
FORCE_INLINE Vec<float, 4> operator/(const U& lhs, const Vec<float, 4>& rhs) {
    __m128 scalar = _mm_set1_ps(lhs);
    return vector_div_sse(scalar, rhs.simd);
}

FORCE_INLINE Vec<float, 4>& operator/=(Vec<float, 4>& lhs, const Vec<float, 4>& rhs) {
    lhs.simd = vector_div_sse(lhs.simd, rhs.simd);
    return lhs;
}

template<Arithmetic U>
FORCE_INLINE Vec<float, 4>& operator/=(Vec<float, 4>& lhs, const U& rhs) {
    __m128 scalar = _mm_set1_ps(rhs);
    lhs.simd = vector_div_sse(lhs.simd, scalar);
    return lhs;
}

}  // namespace cave::math
