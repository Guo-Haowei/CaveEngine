#pragma once
#include <xmmintrin.h>

#include "common.h"
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"

namespace cave {

static_assert(alignof(__m128) == 16);

FORCE_INLINE __m128 vector_add_sse(__m128 p_lhs, __m128 p_rhs) {
    return _mm_add_ps(p_lhs, p_rhs);
}

FORCE_INLINE __m128 vector_sub_sse(__m128 p_lhs, __m128 p_rhs) {
    return _mm_sub_ps(p_lhs, p_rhs);
}

FORCE_INLINE __m128 vector_mul_sse(__m128 p_lhs, __m128 p_rhs) {
    return _mm_mul_ps(p_lhs, p_rhs);
}

FORCE_INLINE __m128 vector_div_sse(__m128 p_lhs, __m128 p_rhs) {
    return _mm_div_ps(p_lhs, p_rhs);
}

// ---- Addition ----

FORCE_INLINE Vector<float, 4> operator+(const Vector<float, 4>& p_lhs, const Vector<float, 4>& p_rhs) {
    return vector_add_sse(p_lhs.simd, p_rhs.simd);
}

template<Arithmetic U>
FORCE_INLINE Vector<float, 4> operator+(const Vector<float, 4>& p_lhs, const U& p_rhs) {
    __m128 scalar = _mm_set1_ps(p_rhs);
    return vector_add_sse(p_lhs.simd, scalar);
}

template<Arithmetic U>
FORCE_INLINE Vector<float, 4> operator+(const U& p_lhs, const Vector<float, 4>& p_rhs) {
    __m128 scalar = _mm_set1_ps(p_lhs);
    return vector_add_sse(scalar, p_rhs.simd);
}

FORCE_INLINE Vector<float, 4>& operator+=(Vector<float, 4>& p_lhs, const Vector<float, 4>& p_rhs) {
    p_lhs.simd = vector_add_sse(p_lhs.simd, p_rhs.simd);
    return p_lhs;
}

template<Arithmetic U>
FORCE_INLINE Vector<float, 4>& operator+=(Vector<float, 4>& p_lhs, const U& p_rhs) {
    __m128 scalar = _mm_set1_ps(p_rhs);
    p_lhs.simd = vector_add_sse(p_lhs.simd, scalar);
    return p_lhs;
}

// ---- Subtraction ----

FORCE_INLINE Vector<float, 4> operator-(const Vector<float, 4>& p_lhs, const Vector<float, 4>& p_rhs) {
    return vector_sub_sse(p_lhs.simd, p_rhs.simd);
}

template<Arithmetic U>
FORCE_INLINE Vector<float, 4> operator-(const Vector<float, 4>& p_lhs, const U& p_rhs) {
    __m128 scalar = _mm_set1_ps(p_rhs);
    return vector_sub_sse(p_lhs.simd, scalar);
}

template<Arithmetic U>
FORCE_INLINE Vector<float, 4> operator-(const U& p_lhs, const Vector<float, 4>& p_rhs) {
    __m128 scalar = _mm_set1_ps(p_lhs);
    return vector_sub_sse(scalar, p_rhs.simd);
}

FORCE_INLINE Vector<float, 4>& operator-=(Vector<float, 4>& p_lhs, const Vector<float, 4>& p_rhs) {
    p_lhs.simd = vector_sub_sse(p_lhs.simd, p_rhs.simd);
    return p_lhs;
}

template<Arithmetic U>
FORCE_INLINE Vector<float, 4>& operator-=(Vector<float, 4>& p_lhs, const U& p_rhs) {
    __m128 scalar = _mm_set1_ps(p_rhs);
    p_lhs.simd = vector_sub_sse(p_lhs.simd, scalar);
    return p_lhs;
}

// ---- Multiplication ----

FORCE_INLINE Vector<float, 4> operator*(const Vector<float, 4>& p_lhs, const Vector<float, 4>& p_rhs) {
    return vector_mul_sse(p_lhs.simd, p_rhs.simd);
}

template<Arithmetic U>
FORCE_INLINE Vector<float, 4> operator*(const Vector<float, 4>& p_lhs, const U& p_rhs) {
    __m128 scalar = _mm_set1_ps(p_rhs);
    return vector_mul_sse(p_lhs.simd, scalar);
}

template<Arithmetic U>
FORCE_INLINE Vector<float, 4> operator*(const U& p_lhs, const Vector<float, 4>& p_rhs) {
    __m128 scalar = _mm_set1_ps(p_lhs);
    return vector_mul_sse(scalar, p_rhs.simd);
}

FORCE_INLINE Vector<float, 4>& operator*=(Vector<float, 4>& p_lhs, const Vector<float, 4>& p_rhs) {
    p_lhs.simd = vector_mul_sse(p_lhs.simd, p_rhs.simd);
    return p_lhs;
}

template<Arithmetic U>
FORCE_INLINE Vector<float, 4>& operator*=(Vector<float, 4>& p_lhs, const U& p_rhs) {
    __m128 scalar = _mm_set1_ps(p_rhs);
    p_lhs.simd = vector_mul_sse(p_lhs.simd, scalar);
    return p_lhs;
}

// ---- DIVISION ----

FORCE_INLINE Vector<float, 4> operator/(const Vector<float, 4>& p_lhs, const Vector<float, 4>& p_rhs) {
    return vector_div_sse(p_lhs.simd, p_rhs.simd);
}

template<Arithmetic U>
FORCE_INLINE Vector<float, 4> operator/(const Vector<float, 4>& p_lhs, const U& p_rhs) {
    __m128 scalar = _mm_set1_ps(p_rhs);
    return vector_div_sse(p_lhs.simd, scalar);
}

template<Arithmetic U>
FORCE_INLINE Vector<float, 4> operator/(const U& p_lhs, const Vector<float, 4>& p_rhs) {
    __m128 scalar = _mm_set1_ps(p_lhs);
    return vector_div_sse(scalar, p_rhs.simd);
}

FORCE_INLINE Vector<float, 4>& operator/=(Vector<float, 4>& p_lhs, const Vector<float, 4>& p_rhs) {
    p_lhs.simd = vector_div_sse(p_lhs.simd, p_rhs.simd);
    return p_lhs;
}

template<Arithmetic U>
FORCE_INLINE Vector<float, 4>& operator/=(Vector<float, 4>& p_lhs, const U& p_rhs) {
    __m128 scalar = _mm_set1_ps(p_rhs);
    p_lhs.simd = vector_div_sse(p_lhs.simd, scalar);
    return p_lhs;
}

}  // namespace cave
