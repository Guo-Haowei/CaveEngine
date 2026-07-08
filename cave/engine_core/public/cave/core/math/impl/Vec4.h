// =============================================================================
// File: cave/core/math/impl/Vec4.h
// =============================================================================
#pragma once
#include "Swizzle.h"
#include "VecBase.h"

namespace cave::math {

template<Arithmetic T>
struct alignas(sizeof(T) * 4) Vec<T, 4> : VectorBase<T, 4> {
    using Base = VectorBase<T, 4>;
    using Self = Vec<T, 4>;

    WARNING_PUSH()
    WARNING_DISABLE(4201, "-Wgnu-anonymous-struct")
    WARNING_DISABLE(4201, "-Wnested-anon-types")
    WARNING_DISABLE(4201, "-Wpadded")
    // clang-format off
    union {
        struct { T x, y, z, w; };
        struct { T r, g, b, a; };
        VECTOR4_SWIZZLE2;
        VECTOR4_SWIZZLE3;
        VECTOR4_SWIZZLE4;
#if USING(MATH_ENABLE_SIMD_SSE)
        __m128 simd;
#elif USING(MATH_ENABLE_SIMD_NEON)
        float16x4_t simd;
#endif
    };
    // clang-format on
    WARNING_POP()

    explicit constexpr Vec() = default;

    explicit constexpr Vec(T v) noexcept
        : x(v), y(v), z(v), w(v) {
    }

    constexpr Vec(T x, T y, T z, T w) noexcept
        : x(x), y(y), z(z), w(w) {
    }

#if USING(MATH_ENABLE_SIMD_SSE)
    constexpr Vec(__m128 simd)
        : simd(simd) {
    }
#elif USING(MATH_ENABLE_SIMD_NEON)
    constexpr Vec(float16x4_t simd)
        : simd(simd) {
    }
#endif

    template<typename U>
        requires Arithmetic<U> && (!std::is_same<T, U>::value)
    explicit constexpr Vec(U x, U y, U z, U w)
        : x(static_cast<T>(x)), y(static_cast<T>(y)), z(static_cast<T>(z)), w(static_cast<T>(w)) {
    }

    template<Arithmetic U>
        requires(!std::is_same<T, U>::value)
    explicit Vec(const Vec<U, 4>& rhs)
        : x(static_cast<T>(rhs.x)), y(static_cast<T>(rhs.y)), z(static_cast<T>(rhs.z)), w(static_cast<T>(rhs.w)) {
    }

    explicit constexpr Vec(const Vec<T, 3>& vec, T w) noexcept
        : x(vec.x), y(vec.y), z(vec.z), w(w) {
    }

    explicit constexpr Vec(const Vec<T, 2>& vec1, const Vec<T, 2>& vec2) noexcept
        : x(vec1.x), y(vec1.y), z(vec2.x), w(vec2.y) {
    }

    explicit constexpr Vec(const Vec<T, 2>& vec, T z, T w) noexcept
        : x(vec.x), y(vec.y), z(z), w(w) {
    }

    template<int N, int A, int B, int C, int D>
    constexpr Vec(const Swizzle4<T, N, A, B, C, D>& swizzle)
        : x(swizzle.d[A]), y(swizzle.d[B]), z(swizzle.d[C]), w(swizzle.d[D]) {
    }

    static const Self Zero;
    static const Self One;
    static const Self UnitX;
    static const Self UnitY;
    static const Self UnitZ;
    static const Self UnitW;
};

template<Arithmetic T>
const Vec<T, 4> Vec<T, 4>::Zero(static_cast<T>(0));
template<Arithmetic T>
const Vec<T, 4> Vec<T, 4>::One(static_cast<T>(1));
template<Arithmetic T>
const Vec<T, 4> Vec<T, 4>::UnitX(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0));
template<Arithmetic T>
const Vec<T, 4> Vec<T, 4>::UnitY(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0), static_cast<T>(0));
template<Arithmetic T>
const Vec<T, 4> Vec<T, 4>::UnitZ(static_cast<T>(0), static_cast<T>(0), static_cast<T>(1), static_cast<T>(0));
template<Arithmetic T>
const Vec<T, 4> Vec<T, 4>::UnitW(static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));

}  // namespace cave::math
