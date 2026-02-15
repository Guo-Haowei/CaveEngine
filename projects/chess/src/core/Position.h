#pragma once
#include <array>
#include <string_view>
#include "Piece.h"
#include "Bitboard.h"

namespace cave::chess {

class Position {
public:
    static Position FromFEN(std::string_view p_fen);

private:
    Color m_side_to_move = Color::White;
    std::array<Bitboard, kPieceMax> m_boards;
};

}  // namespace cave::chess
