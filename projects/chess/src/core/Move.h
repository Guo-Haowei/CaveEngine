#pragma once
#include <string>
#include <vector>
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
    Square from{ 0 };
    Square to{ 0 };

    MoveType GetType() const { return MoveType::Normal; }

    std::string Uci() const;
};

using MoveList = std::vector<Move>;

}  // namespace chess::core
