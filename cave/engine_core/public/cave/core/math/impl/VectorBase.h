// =============================================================================
// File: cave/core/math/impl/VectorBase.h
// =============================================================================
#pragma once
#include "Forward.h"

namespace cave::math {

template<Arithmetic T, int N>
    requires(N >= 2 && N <= 4)
struct VectorBase {
    using Self = VectorBase<T, N>;

    constexpr T* data() { return reinterpret_cast<T*>(this); }
    constexpr const T* data() const { return reinterpret_cast<const T*>(this); }

    constexpr void set(const T* src) {
        T* data = reinterpret_cast<T*>(this);
        data[0] = src[0];
        data[1] = src[1];
        if constexpr (N >= 3) {
            data[2] = src[2];
        }
        if constexpr (N >= 4) {
            data[3] = src[3];
        }
    }

    constexpr T& operator[](size_t idx) {
        return data()[idx];
    }

    constexpr const T& operator[](size_t p_index) const {
        return data()[p_index];
    }
};

}  // namespace cave::math
