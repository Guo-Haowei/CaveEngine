#include "cave/core/Color.h"

namespace cave {

uint32_t Color::ToRgb() const {
    uint32_t c = (uint8_t)std::round(r * 255.0f);
    c <<= 8;
    c |= (uint8_t)std::round(g * 255.0f);
    c <<= 8;
    c |= (uint8_t)std::round(b * 255.0f);
    return c;
}

uint32_t Color::ToRgba() const {
    uint32_t c = (uint8_t)std::round(r * 255.0f);
    c <<= 8;
    c |= (uint8_t)std::round(g * 255.0f);
    c <<= 8;
    c |= (uint8_t)std::round(b * 255.0f);
    c <<= 8;
    c |= (uint8_t)std::round(a * 255.0f);
    return c;
}

Color Color::HexRgba(ColorCode p_hex) {
    uint32_t hex = std::to_underlying(p_hex);
    const float a = (hex & 0xFF) / 255.0f;
    hex >>= 8;
    const float b = (hex & 0xFF) / 255.0f;
    hex >>= 8;
    const float g = (hex & 0xFF) / 255.0f;
    hex >>= 8;
    const float r = (hex & 0xFF) / 255.0f;
    return Color(r, g, b, a);
}

}  // namespace cave
