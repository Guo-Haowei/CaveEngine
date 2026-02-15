#pragma once
#include <array>
#include <cstdint>

namespace cave {

enum class PieceColor : uint8_t {
    White,
    Black,
    Null,
};

enum class PieceType : uint8_t {
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,

    _Count,
    Null,
};

enum class Piece : uint8_t {
    WP,
    WN,
    WB,
    WR,
    WQ,
    WK,
    BP,
    BN,
    BB,
    BR,
    BQ,
    BK,

    Null,
};

static constexpr PieceType GetType(Piece p_piece) {
    if (p_piece == Piece::Null) return PieceType::Null;
    constexpr uint8_t kPieceTypeCount = std::to_underlying(PieceType::_Count);
    const uint8_t type = std::to_underlying(p_piece) % kPieceTypeCount;
    return static_cast<PieceType>(type);
}

static constexpr PieceColor GetColor(Piece p_piece) {
    if (p_piece == Piece::Null) return PieceColor::Null;
    constexpr uint8_t kPieceTypeCount = std::to_underlying(PieceType::_Count);
    const uint8_t type = (std::to_underlying(p_piece)) / kPieceTypeCount;
    return static_cast<PieceColor>(type);
}

static_assert(GetType(Piece::Null) == PieceType::Null);
static_assert(GetType(Piece::WP) == PieceType::Pawn);
static_assert(GetType(Piece::WN) == PieceType::Knight);
static_assert(GetType(Piece::WB) == PieceType::Bishop);
static_assert(GetType(Piece::WR) == PieceType::Rook);
static_assert(GetType(Piece::WQ) == PieceType::Queen);
static_assert(GetType(Piece::WK) == PieceType::King);
static_assert(GetType(Piece::BP) == PieceType::Pawn);
static_assert(GetType(Piece::BN) == PieceType::Knight);
static_assert(GetType(Piece::BB) == PieceType::Bishop);
static_assert(GetType(Piece::BR) == PieceType::Rook);
static_assert(GetType(Piece::BQ) == PieceType::Queen);
static_assert(GetType(Piece::BK) == PieceType::King);

static_assert(GetColor(Piece::Null) == PieceColor::Null);
static_assert(GetColor(Piece::WP) == PieceColor::White);
static_assert(GetColor(Piece::WN) == PieceColor::White);
static_assert(GetColor(Piece::WB) == PieceColor::White);
static_assert(GetColor(Piece::WR) == PieceColor::White);
static_assert(GetColor(Piece::WQ) == PieceColor::White);
static_assert(GetColor(Piece::WK) == PieceColor::White);
static_assert(GetColor(Piece::BP) == PieceColor::Black);
static_assert(GetColor(Piece::BN) == PieceColor::Black);
static_assert(GetColor(Piece::BB) == PieceColor::Black);
static_assert(GetColor(Piece::BR) == PieceColor::Black);
static_assert(GetColor(Piece::BQ) == PieceColor::Black);
static_assert(GetColor(Piece::BK) == PieceColor::Black);

}  // namespace cave
