#include "Position.h"

namespace chess::core {

Position Position::FromFEN(std::string_view p_fen) {
    (void)p_fen;
    Position pos;
    return pos;
}

#if defined(CAVE_TEST)

TEST(Position, get_side_to_move) {
    Position pos;
    EXPECT_EQ(pos.SideToMove(), Color::White);
}

#endif

}  // namespace chess::core