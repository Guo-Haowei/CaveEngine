#include "Piece.h"

#include <cassert>

namespace chess::core {

const char* GetPieceTypeName(PieceType p_type) {
    static constexpr const char* kPieceNameTable[kPieceTypeMax]{
        "pawn",
        "knight",
        "bishop",
        "rook",
        "queen",
        "king",
    };
    assert((uint32_t)p_type < kPieceTypeMax);
    return kPieceNameTable[std::to_underlying(p_type)];
}

const char* GetPieceName(Piece p_piece) {
    static constexpr const char* kPieceNameTable[kPieceMax]{
        "wP",
        "wN",
        "wB",
        "wR",
        "wQ",
        "wK",
        "bP",
        "bN",
        "bB",
        "bR",
        "bQ",
        "bK",
    };
    assert((uint32_t)p_piece < kPieceMax);
    return kPieceNameTable[std::to_underlying(p_piece)];
}

static_assert(GetType(Piece::Null) == PieceType::Null);
static_assert(GetType(Piece::WP) == PieceType::Pawn);
static_assert(GetType(Piece::WB) == PieceType::Bishop);
static_assert(GetType(Piece::WK) == PieceType::King);
static_assert(GetType(Piece::BP) == PieceType::Pawn);
static_assert(GetType(Piece::BN) == PieceType::Knight);
static_assert(GetType(Piece::BK) == PieceType::King);

static_assert(GetColor(Piece::Null) == Color::Null);
static_assert(GetColor(Piece::WP) == Color::White);
static_assert(GetColor(Piece::WQ) == Color::White);
static_assert(GetColor(Piece::WK) == Color::White);
static_assert(GetColor(Piece::BP) == Color::Black);
static_assert(GetColor(Piece::BB) == Color::Black);
static_assert(GetColor(Piece::BK) == Color::Black);

}  // namespace chess::core
