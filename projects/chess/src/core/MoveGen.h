#pragma once
#include <vector>
#include "Move.h"
#include "Piece.h"

namespace chess::core {

class Position;

struct MoveGen {
    static MoveList Pseudo(const Position& p_pos);

    static void PseudoFromSquare(const Position& p_pos,
                           Square p_from,
                           Piece p_piece,
                           MoveList& p_move_list);
};

}  // namespace chess::core
