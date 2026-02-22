#pragma once
#include <vector>
#include "Move.h"
#include "Piece.h"

namespace chess::core {

class Position;

struct MoveGen {
    static void Pseudo(const Position& p_pos,
                       MoveList& p_move_list);

    static void PseudoFromSquare(const Position& p_pos,
                           Square p_from,
                           Piece p_piece,
                           MoveList& p_move_list);
};

}  // namespace chess::core
