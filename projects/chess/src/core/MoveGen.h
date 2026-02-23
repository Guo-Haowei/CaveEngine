#pragma once
#include <vector>
#include "Bitboard.h"
#include "CheckerList.h"
#include "Move.h"
#include "Piece.h"

namespace chess::core {

class Position;

class MoveGen {
public:
    static MoveList Pseudo(const Position& p_pos);

    static void AttackMapAndCheckers(const Position& p_pos,
                                     Color p_color,
                                     Bitboard& p_out_attack,
                                     CheckerList& p_out_checkers);

private:
    static void PseudoFromSquare(const Position& p_pos,
                                 Square p_from,
                                 Piece p_piece,
                                 MoveList& p_move_list,
                                 Square p_king_sq,
                                 bool p_in_check,
                                 Square p_checker_sq,
                                 PieceType p_checker_type);
};

}  // namespace chess::core
