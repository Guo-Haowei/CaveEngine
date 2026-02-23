#pragma once
#include <string>
#include <vector>
#include "Piece.h"
#include "Square.h"

namespace chess::core {

enum class MoveType : uint8_t {
    Normal = 0,
    Castling = 1,
    EnPassant = 2,
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

    explicit Move(Square p_from, Square p_to, MoveType p_type, PieceType p_promotion) noexcept;

    explicit Move(Square p_from, Square p_to, MoveType p_type) noexcept
        : Move(p_from, p_to, p_type, PieceType::Null) {}

    MoveType GetType() const { return static_cast<MoveType>(m_flag); }

    std::string Uci() const;

    Square From() const { return Square((uint8_t)m_from); }
    Square To() const { return Square((uint8_t)m_to); }

    bool operator==(const Move& p_rhs) const = default;

    bool IsValid() const { return *this != Null(); }

    static Move Null() {
        return Move();
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
    uint32_t m_count{0};
};

}  // namespace chess::core
