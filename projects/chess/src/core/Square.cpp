#include "Square.h"

#include "cave/core/ErrorMacros.h"

namespace chess::core {

std::tuple<uint8_t, uint8_t> Square::FileRank() const {
    const uint8_t file = m_index & 7;
    const uint8_t rank = m_index >> 3;
    return std::make_tuple(file, rank);
}

const char* Square::ToString() const {
    static constexpr const char kSquareLookUp[64][3] = {
        // clang-format off
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
        // clang-format on
    };

    DEV_ASSERT_INDEX(m_index, 64);
    return kSquareLookUp[m_index];
}

}  // namespace chess::core
