#pragma once
#include <string>
#include <vector>
#include "cave/core/Option.h"
#include "Piece.h"
#include "Square.h"

namespace chess::core {

enum class MoveType : uint8_t {
    Normal = 0,
    Castling = 1,
    Enpassant = 2,
    Promotion = 3,
};

/// Bit layout for a `Move` (16-bit packed):
///
/// ```text
/// 15  14  13  12   11        6   5        0
/// +---+---+---+---+----------+------------+
/// | P | P | F | F |  To[5:0] | From[5:0]  |
/// +---+---+---+---+----------+------------+
///  2 bits  2 bits    6 bits     6 bits
///  [14-15] [12-13]   [6–11]     [0–5]
/// ```
///
/// - `from` (0–5): source square (0–63)
/// - `to` (6–11): destination square (0–63)
/// - `flag` (12–13): move type (e.g., castle, en passant, promotion)
/// - `promo` (14–15): promotion piece (0 = knight, 1 = bishop, 2 = rook, 3 = queen)
class Move {
public:
    Move() noexcept;

    Move(Square from, Square to, MoveType type, PieceType promo) noexcept;

    MoveType type() const { return static_cast<MoveType>(flag_); }

    cave::Option<PieceType> promo() const;

    std::string uci() const;

    Square from() const { return Square((uint8_t)from_); }
    Square to() const { return Square((uint8_t)to_); }

    bool operator==(const Move& rhs) const = default;

    bool valid() const { return *this != null(); }

    static Move null() {
        return Move();
    }

    static Move normal(Square from, Square to) {
        return Move(from, to, MoveType::Normal, PieceType::Null);
    }

    static Move castle(Square from, Square to) {
        return Move(from, to, MoveType::Castling, PieceType::Null);
    }

    static Move enpassant(Square from, Square to) {
        return Move(from, to, MoveType::Enpassant, PieceType::Null);
    }

    static Move promotion(Square from, Square to, PieceType type) {
        return Move(from, to, MoveType::Promotion, type);
    }

private:
    uint16_t from_ : 6;
    uint16_t to_ : 6;
    uint16_t flag_ : 2;
    uint16_t promo_ : 2;
};

class MoveList {
public:
    void addMove(Move move);
    void clear();

    bool empty() const { return size_ == 0; }
    uint32_t size() const { return size_; }

    Move* begin() { return moves_.data(); }
    Move* end() { return moves_.data() + size_; }

    const Move* begin() const { return moves_.data(); }
    const Move* end() const { return moves_.data() + size_; }

    Move& operator[](size_t idx) { return moves_[idx]; }
    const Move& operator[](size_t idx) const { return moves_[idx]; }

private:
    std::array<Move, 256> moves_{};
    uint32_t size_{ 0 };
};

}  // namespace chess::core
