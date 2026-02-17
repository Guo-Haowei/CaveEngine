#include "Piece.h"

#include <cassert>

#include "cave/core/containers/EnumArray.h"

namespace chess::core {

Piece ParsePieceChar(char p_char) {
    // clang-format off
    switch (p_char) {
        case 'P': return Piece::WP;
        case 'N': return Piece::WN;
        case 'B': return Piece::WB;
        case 'R': return Piece::WR;
        case 'Q': return Piece::WQ;
        case 'K': return Piece::WK;
        case 'p': return Piece::BP;
        case 'n': return Piece::BN;
        case 'b': return Piece::BB;
        case 'r': return Piece::BR;
        case 'q': return Piece::BQ;
        case 'k': return Piece::BK;
        default: return Piece::Null;
    }
    // clang-format on
}

char GetPieceChar(Piece p_piece) {
    static constexpr cave::EnumArray<Piece, char, kPieceMax + 1> kCharTable{
        'P',
        'N',
        'B',
        'R',
        'Q',
        'K',
        'p',
        'n',
        'b',
        'r',
        'q',
        'k',
        '.',
    };
    return kCharTable[p_piece];
}

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
