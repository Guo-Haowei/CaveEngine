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

    Move(Square p_from, Square p_to, MoveType p_type, PieceType p_promotion) noexcept;

    MoveType GetType() const { return static_cast<MoveType>(m_flag); }

    cave::Option<PieceType> GetPromo() const;

    std::string Uci() const;

    Square From() const { return Square((uint8_t)m_from); }
    Square To() const { return Square((uint8_t)m_to); }

    bool operator==(const Move& p_rhs) const = default;

    bool IsValid() const { return *this != Null(); }

    static Move Null() {
        return Move();
    }

    static Move Normal(Square from, Square to) {
        return Move(from, to, MoveType::Normal, PieceType::Null);
    }

    static Move Castle(Square from, Square to) {
        return Move(from, to, MoveType::Castling, PieceType::Null);
    }

    static Move Enpassant(Square from, Square to) {
        return Move(from, to, MoveType::Enpassant, PieceType::Null);
    }

    static Move Promotion(Square from, Square to, PieceType type) {
        return Move(from, to, MoveType::Promotion, type);
    }

private:
    uint16_t m_from : 6;
    uint16_t m_to : 6;
    uint16_t m_flag : 2;
    uint16_t m_promo : 2;
};

class MoveList {
public:
    void Add(Move p_move);
    void Clear();

    bool IsEmpty() const { return m_count == 0; }
    uint32_t Size() const { return m_count; }

    Move* begin() { return m_moves.data(); }
    Move* end() { return m_moves.data() + m_count; }

    const Move* begin() const { return m_moves.data(); }
    const Move* end() const { return m_moves.data() + m_count; }

private:
    std::array<Move, 256> m_moves{};
    uint32_t m_count{ 0 };
};

}  // namespace chess::core
