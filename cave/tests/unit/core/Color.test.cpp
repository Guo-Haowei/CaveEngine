#include "cave/core/Color.h"

namespace cave {

TEST(color, to_rgb) {
    Color c = Color::Hex(ColorCode::Red);
    uint32_t rgb = c.ToRgb();
    EXPECT_EQ(rgb, std::to_underlying(ColorCode::Red));
}

}  // namespace cave
