// =============================================================================
// File: cave/core/Color.h
// =============================================================================
#pragma once
#include "cave/core/math/Vector.h"

namespace cave {

enum class ColorCode : uint32_t {
    Green = 0x80E080,
    Yellow = 0xE0E080,
    Red = 0xE08080,
    Palegreen = 0x98FB98,
    Silver = 0xA0A0A0,
    White = 0xE0E0E0,
};

class Color : public math::Vector<float, 4> {
    using Base = math::Vector<float, 4>;

public:
    constexpr Color()
        : Base(0, 0, 0, 1) {}
    constexpr Color(float p_r, float p_g, float p_b, float p_a)
        : Base(p_r, p_g, p_b, p_a) {}
    constexpr Color(float p_r, float p_g, float p_b)
        : Base(p_r, p_g, p_b, 1.0f) {}

    uint32_t ToRgb() const;
    uint32_t ToRgba() const;

    math::Vec4f ToVec4f() const {
        return math::Vec4f(r, g, b, a);
    }

    static constexpr Color Hex(ColorCode p_hex) {
        uint32_t hex = std::to_underlying(p_hex);
        const float b = (hex & 0xFF) / 255.0f;
        hex >>= 8;
        const float g = (hex & 0xFF) / 255.0f;
        hex >>= 8;
        const float r = (hex & 0xFF) / 255.0f;
        return Color(r, g, b, 1.0f);
    }

    static Color HexRgba(ColorCode p_hex);
};

}  // namespace cave
