#pragma once
#include <array>
#include <cstdint>

namespace cave::chess {

enum class Color : uint8_t {
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

constexpr uint8_t kColorMax = std::to_underlying(Color::Null);
constexpr uint8_t kPieceMax = std::to_underlying(Piece::Null);
constexpr uint8_t kPieceTypeMax = std::to_underlying(PieceType::Null);

static constexpr PieceType GetType(Piece p_piece) {
    if (p_piece == Piece::Null) return PieceType::Null;
    const uint8_t type = std::to_underlying(p_piece) % kPieceTypeMax;
    return static_cast<PieceType>(type);
}

static constexpr Color GetColor(Piece p_piece) {
    if (p_piece == Piece::Null) return Color::Null;
    const uint8_t type = (std::to_underlying(p_piece)) / kPieceTypeMax;
    return static_cast<Color>(type);
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

}  // namespace cave::chess
