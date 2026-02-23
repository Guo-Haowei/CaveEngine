#pragma once
#include <string>
#include <vector>
#include "Piece.h"
#include "Square.h"

namespace chess::core {

enum class MoveType {
    Normal = 0,
    Castling = 1,
    EnPassant = 2,
    Promotion = 3,
};

// @TODO: pack move
struct Move {
    constexpr Move() noexcept {}

    explicit constexpr Move(Square p_from, Square p_to, MoveType p_type, PieceType p_promotion) noexcept
        : from(p_from)
        , to(p_to) {
        (void)p_type;
        (void)p_promotion;
    }

    Square from{ 0 };
    Square to{ 0 };

    MoveType GetType() const { return MoveType::Normal; }

    std::string Uci() const;
};

using MoveList = std::vector<Move>;

}  // namespace chess::core
