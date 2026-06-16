// =============================================================================
// File: cave/core/math/impl/Vector3.h
// =============================================================================
#pragma once
#include "Swizzle.h"
#include "VectorBase.h"

namespace cave::math {

template<Arithmetic T>
struct Vector<T, 3> : VectorBase<T, 3> {
    using Base = VectorBase<T, 3>;
    using Self = Vector<T, 3>;

    WARNING_PUSH()
    WARNING_DISABLE(4201, "-Wgnu-anonymous-struct")
    WARNING_DISABLE(4201, "-Wnested-anon-types")
    WARNING_DISABLE(4201, "-Wpadded")
    // clang-format off
    union {
        struct { T x, y, z; };
        struct { T r, g, b; };

        VECTOR3_SWIZZLE2;
        VECTOR3_SWIZZLE3;
    };
    // clang-format on
    WARNING_POP()

    explicit constexpr Vector() = default;

    explicit constexpr Vector(T v) noexcept
        : x(v), y(v), z(v) {
    }

    constexpr Vector(T x, T y, T z) noexcept
        : x(x), y(y), z(z) {
    }

    template<Arithmetic U>
        requires(!std::is_same<T, U>::value)
    explicit constexpr Vector(U x, U y, U z)
        : x(static_cast<T>(x)), y(static_cast<T>(y)), z(static_cast<T>(z)) {
    }

    template<Arithmetic U>
        requires(!std::is_same<T, U>::value)
    explicit Vector(const Vector<U, 3>& rhs)
        : x(static_cast<T>(rhs.x)), y(static_cast<T>(rhs.y)), z(static_cast<T>(rhs.z)) {
    }

    explicit constexpr Vector(const Vector<T, 2>& vec, T z) noexcept
        : x(vec.x), y(vec.y), z(z) {
    }

    template<int N, int A, int B, int C>
    constexpr Vector(const Swizzle3<T, N, A, B, C, -1>& swizzle)
        : x(swizzle.d[A]), y(swizzle.d[B]), z(swizzle.d[C]) {
    }

    static const Self Zero;
    static const Self One;
    static const Self UnitX;
    static const Self UnitY;
    static const Self UnitZ;
};

template<Arithmetic T>
const Vector<T, 3> Vector<T, 3>::Zero(static_cast<T>(0));
template<Arithmetic T>
const Vector<T, 3> Vector<T, 3>::One(static_cast<T>(1));
template<Arithmetic T>
const Vector<T, 3> Vector<T, 3>::UnitX(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0));
template<Arithmetic T>
const Vector<T, 3> Vector<T, 3>::UnitY(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0));
template<Arithmetic T>
const Vector<T, 3> Vector<T, 3>::UnitZ(static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));

}  // namespace cave::math
