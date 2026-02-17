#pragma once
#include <array>
#include <expected>
#include <string_view>

#include "cave/core/Option.h"
#include "cave/core/containers/EnumArray.h"

#include "Piece.h"
#include "Bitboard.h"

namespace chess::core {

enum class Castling {
    WK,
    WQ,
    BK,
    BQ,
};

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
    Castling castling;
    cave::Option<Square> en_passant;
    uint32_t halfmove_clock;
    uint32_t fullmove_number;

    cave::EnumArray<Color, Bitboard, 3> occupancies;
    cave::EnumArray<Color, Bitboard, 2> attack_mask;

    // @TODO: captured piece
    // pub captured_piece: Piece,
    // pub checkers: [CheckerList; Color::COUNT],
    // pub king_squares: [Square; Color::COUNT],
};

class Position {
public:
    using Board = cave::EnumArray<Piece, Bitboard, kPieceMax>;

    Position() = default;

    Color SideToMove() const { return m_side_to_move; }

    static Position Default();
    static std::expected<Position, FenError> FromFen(std::string_view p_fen);

    Piece PieceAt(Square p_sq) const;

    Color ColorAt(Square p_sq) const;

    std::string Fen() const;

    std::string DebugBoardString() const;

private:
    void UpdateCache();

    Board m_board{};
    Color m_side_to_move{ Color::White };
    UndoState m_state;
};

#if 0
impl Position {
    pub const DEFAULT_FEN: &str = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    pub fn new() -> Self {
        Self::from_fen(Self::DEFAULT_FEN).unwrap()
    }

    pub fn from_fen(fen: &str) -> Result<Self, &'static str> {
        let parts: Vec<&str> = fen.trim().split_whitespace().collect();
        if parts.len() != 6 {
            return Err("Invalid FEN: must have 6 fields");
        }

        let bitboards = utils::parse_board(parts[0])?;
        let side_to_move = match Color::parse(parts[1]) {
            Some(color) => color,
            None => return Err("Invalid side to move in FEN"),
        };
        let castling = utils::parse_castling(parts[2])?;

        let en_passant = utils::parse_en_passant(parts[3]);
        if en_passant.is_none() {
            return Err("Invalid en passant square in FEN");
        }
        let en_passant = en_passant.unwrap();

        let halfmove_clock = utils::parse_halfmove_clock(parts[4])?;
        let fullmove_number = utils::parse_fullmove_number(parts[5])?;

        let state = UndoState {
            castling_rights: castling,
            en_passant,
            halfmove_clock,
            fullmove_number,
            captured_piece: Piece::NONE,
            occupancies: [BitBoard::new(); 3],
            attack_mask: [BitBoard::new(); Color::COUNT],
            checkers: [CheckerList::new(); Color::COUNT],
            king_squares: [Square::NONE; Color::COUNT],
        };

        let mut pos = Position { bitboards, side_to_move, state };
        internal::update_cache(&mut pos);

        Ok(pos)
    }

    pub fn fen(&self) -> String {
        format!(
            "{} {} {} {} {} {}",
            utils::dump_board(&self.bitboards),
            if self.white_to_move() { "w" } else { "b" },
            utils::dump_castling(self.state.castling_rights),
            match self.state.en_passant {
                Some(sq) => sq.to_string(),
                None => "-".to_string(),
            },
            self.state.halfmove_clock,
            self.state.fullmove_number
        )
    }

    pub fn zobrist(&self) -> ZobristHash {
        zobrist_hash(&self)
    }

    pub fn white_to_move(&self) -> bool {
        self.side_to_move == Color::WHITE
    }

    pub fn get_piece_at(&self, sq: Square) -> Piece {
        let sq = sq.as_u8();
        if self.state.occupancies[Color::BOTH.as_usize()].test(sq) == false {
            return Piece::NONE;
        }

        let color = if self.state.occupancies[Color::WHITE.as_usize()].test(sq) {
            Color::WHITE
        } else {
            Color::BLACK
        };

        for i in 0..PieceType::COUNT {
            let piece = Piece::get_piece(color, PieceType(i));
            if self.bitboards[piece.as_usize()].test(sq) {
                return piece;
            }
        }
        panic!("No piece found at square {}", sq);
    }

    pub fn get_color_at(&self, sq: Square) -> Color {
        let is_white = self.state.occupancies[Color::WHITE.as_usize()].test(sq.as_u8());
        let is_black = self.state.occupancies[Color::BLACK.as_usize()].test(sq.as_u8());
        if cfg!(debug_assertions) {
            debug_assert!(is_white ^ is_black, "Square {} has both colors", sq);
            let piece = self.get_piece_at(sq);
            let debug_color = piece.color();
            debug_assert!(
                (is_white && debug_color == Color::WHITE)
                    || (is_black && debug_color == Color::BLACK),
                "Square {} has color {:?}, but piece is {:?}",
                sq,
                debug_color,
                piece
            );
        }

        if !is_white && !is_black {
            debug_assert!(self.state.occupancies[Color::BOTH.as_usize()].test(sq.as_u8()) == false);
            return Color::NONE;
        }

        if is_white { Color::WHITE } else { Color::BLACK }
    }

    pub fn get_king_square(&self, color: Color) -> Square {
        self.state.king_squares[color.as_usize()]
    }

    pub fn is_in_check(&self, color: Color) -> bool {
        let checker_count = self.state.checkers[color.as_usize()].count();

        if cfg!(debug_assertions) && checker_count != 0 {
            let king_sq = self.get_king_square(color);
            let attack_map = self.state.attack_mask[color.flip().as_usize()];
            debug_assert!(
                attack_map.test(king_sq.as_u8()),
                "King square {} is not attacked by opponent's pieces",
                king_sq
            );
        }

        checker_count != 0
    }

    pub fn make_move(&mut self, mv: Move) -> (UndoState, bool) {
        internal::make_move(self, mv)
    }

    pub fn unmake_move(&mut self, mv: Move, undo_state: &UndoState) {
        internal::unmake_move(self, mv, undo_state)
    }
}
#endif

}  // namespace chess::core
