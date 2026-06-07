#pragma once
#include <array>
#include <expected>
#include <string_view>

#include "cave/core/containers/EnumArray.h"
#include "cave/core/typedefs.h"

#include "Bitboard.h"
#include "CheckerList.h"
#include "Move.h"
#include "Piece.h"

namespace chess::core {

enum class CastlingRight : uint8_t {
    None = 0,
    K = 1,
    Q = 2,
    k = 4,
    q = 8,
    KQ = K | Q,
    kq = k | q,
};
DEFINE_ENUM_BITWISE_OPERATIONS(CastlingRight);

enum class FenError {
    Ok,
    InvalidFieldCount,
    InvalidBoard,
    InvalidSideToMove,
    InvalidCastling,
    InvalidEnPassant,
    InvalidHalfmove,
    InvalidFullmove,
};

struct UndoState {
    CastlingRight castling{ CastlingRight::None };
    cave::Option<Square> ep{};
    uint32_t halfmove_clock;
    uint32_t fullmove_number;

    Piece captured_piece{ Piece::Null };

    cave::EnumArray<Color, Bitboard, 3> occupancies;
    cave::EnumArray<Color, Bitboard, 2> attack_mask;

    cave::EnumArray<Color, Square, 2> king_squares;
    cave::EnumArray<Color, CheckerList, 2> checkers;
};

class Position {
public:
    using Board = cave::EnumArray<Piece, Bitboard, kPieceMax>;

    Position() = default;

    Color SideToMove() const { return m_side_to_move; }

    static Position Startpos();
    static std::expected<Position, FenError> FromFen(std::string_view p_fen);

    Piece PieceAt(Square p_sq) const;
    Color ColorAt(Square p_sq) const;

    bool MakeMove(Move p_mv, UndoState& p_undo);
    bool UnmakeMove(Move p_mv, UndoState& p_undo);

    std::string Fen() const;

    std::string DebugBoardString() const;

    Bitboard Bitboard(Piece p_piece) const { return m_board[p_piece]; }
    const UndoState& State() const { return m_state; }

    Square GetKing(Color p_color) const { return m_state.king_squares[p_color]; }

private:
    bool UpdateCache();

    Board m_board{};
    Color m_side_to_move{ Color::White };
    UndoState m_state;

    friend class MoveGen;
};

}  // namespace chess::core
