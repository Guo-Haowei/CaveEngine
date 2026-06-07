// =============================================================================
// File: cave/core/math/impl/Forward.h
// =============================================================================
#pragma once
#include "cave/core/typedefs.h"

#define MATH_ENABLE_SIMD_SSE  NOT_IN_USE
#define MATH_ENABLE_SIMD_NEON NOT_IN_USE

#if USING(ARCH_X64)
#undef MATH_ENABLE_SIMD_SSE
#define MATH_ENABLE_SIMD_SSE IN_USE
#include <xmmintrin.h>
#elif USING(ARCH_ARM64)  // #if USING(ARCH_X64)
#undef MATH_ENABLE_SIMD_NEON
#define MATH_ENABLE_SIMD_NEON IN_USE
#include <arm_neon.h>
#endif  // #else // #if USING(MATH_ENABLE_SIMD)

namespace cave::math {

template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;
template<typename T>
concept FloatingPoint = std::is_floating_point_v<T>;

template<Arithmetic T, int N>
    requires(N >= 2 && N <= 4)
struct VectorBase;

template<Arithmetic T, int N>
struct Vector;

template<Arithmetic T>
struct Vector<T, 2>;

template<Arithmetic T>
struct Vector<T, 3>;

template<Arithmetic T>
struct Vector<T, 4>;

template<typename T, int S, int N, int A, int B, int C, int D>
struct Swizzle;

template<typename T, int N, int A, int B, int C, int D>
using Swizzle2 = Swizzle<T, 2, N, A, B, C, D>;
template<typename T, int N, int A, int B, int C, int D>
using Swizzle3 = Swizzle<T, 3, N, A, B, C, D>;
template<typename T, int N, int A, int B, int C, int D>
using Swizzle4 = Swizzle<T, 4, N, A, B, C, D>;

}  // namespace cave::math
