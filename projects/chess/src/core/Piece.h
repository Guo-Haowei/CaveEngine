#pragma once
#include <array>
#include <cstdint>

namespace chess::core {

enum class Color : uint8_t {
    White,
    Black,
    Null,
    Both = Null,
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

Piece ParsePieceChar(char p_char);
char GetPieceChar(Piece p_piece);
const char* GetPieceTypeName(PieceType p_type);
const char* GetPieceName(Piece p_piece);

constexpr uint8_t kColorMax = std::to_underlying(Color::Null);
constexpr uint8_t kPieceMax = std::to_underlying(Piece::Null);
constexpr uint8_t kPieceTypeMax = std::to_underlying(PieceType::Null);

static constexpr Color FlipColor(Color p_color) {
    if (p_color == Color::White) return Color::Black;
    if (p_color == Color::Black) return Color::White;
    return Color::Null;
}

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

}  // namespace chess::core
