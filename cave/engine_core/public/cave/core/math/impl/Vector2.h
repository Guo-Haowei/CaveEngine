// =============================================================================
// File: cave/core/math/impl/Vector2.h
// =============================================================================
#pragma once
#include "Swizzle.h"
#include "VectorBase.h"

namespace cave::math {

template<Arithmetic T>
struct Vector<T, 2> : VectorBase<T, 2> {
    using Base = VectorBase<T, 2>;
    using Self = Vector<T, 2>;

    WARNING_PUSH()
    WARNING_DISABLE(4201, "-Wgnu-anonymous-struct")
    WARNING_DISABLE(4201, "-Wnested-anon-types")
    WARNING_DISABLE(4201, "-Wpadded")
    // clang-format off
    union {
        struct { T x, y; };
        struct { T r, g; };

        VECTOR2_SWIZZLE2;
    };
    // clang-format on
    WARNING_POP()

    explicit constexpr Vector() = default;

    explicit constexpr Vector(T v) noexcept
        : x(v), y(v) {
    }

    constexpr Vector(T x, T y) noexcept
        : x(x), y(y) {
    }

    template<Arithmetic U>
        requires(!std::is_same<T, U>::value)
    explicit constexpr Vector(U x, U y)
        : x(static_cast<T>(x)), y(static_cast<T>(y)) {
    }

    template<Arithmetic U>
        requires(!std::is_same<T, U>::value)
    explicit Vector(const Vector<U, 2>& rhs)
        : x(static_cast<T>(rhs.x)), y(static_cast<T>(rhs.y)) {
    }

    template<int N, int A, int B>
    constexpr Vector(const Swizzle2<T, N, A, B, -1, -1>& swizzle)
        : x(swizzle.d[A]), y(swizzle.d[B]) {
    }

    static const Self Zero;
    static const Self One;
    static const Self UnitX;
    static const Self UnitY;
};

template<Arithmetic T>
const Vector<T, 2> Vector<T, 2>::Zero(static_cast<T>(0));
template<Arithmetic T>
const Vector<T, 2> Vector<T, 2>::One(static_cast<T>(1));
template<Arithmetic T>
const Vector<T, 2> Vector<T, 2>::UnitX(static_cast<T>(1), static_cast<T>(0));
template<Arithmetic T>
const Vector<T, 2> Vector<T, 2>::UnitY(static_cast<T>(0), static_cast<T>(1));

}  // namespace cave::math
