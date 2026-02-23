#include "Position.h"

#include <cassert>
#include "cave/core/typedefs.h"

#include "MoveGen.h"

namespace chess::core {

using cave::unused;

static constexpr const char kDefaultFen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

static bool SplitFen6(std::string_view fen,
                      std::string_view& board,
                      std::string_view& stm,
                      std::string_view& castling,
                      std::string_view& ep,
                      std::string_view& half,
                      std::string_view& full);

static bool ParseBoard(std::string_view board,
                       cave::EnumArray<Piece, Bitboard, kPieceMax>& p_out);

static std::string DebugBoardString(const Position::Board& p_board);

Piece Position::PieceAt(Square p_sq) const {
    const bool is_white = m_state.occupancies[Color::White].Test(p_sq);
    const bool is_black = m_state.occupancies[Color::Black].Test(p_sq);

    if (!is_white && !is_black) return Piece::Null;

    const uint8_t offset = is_black ? kPieceTypeMax : 0;
    for (uint8_t i = 0; i < kPieceTypeMax; ++i) {
        const Piece piece = static_cast<Piece>(i + offset);
        if (m_board[piece].Test(p_sq)) {
            return piece;
        }
    }

    CRASH_NOW_MSG("should not reach here");
    return Piece::Null;
}

Color Position::ColorAt(Square p_sq) const {
    if (m_state.occupancies[Color::White].Test(p_sq)) {
        return Color::White;
    }
    if (m_state.occupancies[Color::Black].Test(p_sq)) {
        return Color::Black;
    }
    return Color::Null;
}

static void MovePiece(Bitboard& p_board, Square p_src, Square p_to) {
    DEV_ASSERT_MSG(p_board.Test(p_src), "No piece found on 'src' square");
    p_board.Unset(p_src);
    p_board.Set(p_to);
}

bool Position::MakeMove(Move p_move, UndoState& p_undo) {
    const Square src_sq = p_move.from;
    const Square dst_sq = p_move.to;

    const Piece src_piece = PieceAt(src_sq);
    const Piece dst_piece = PieceAt(dst_sq);

    const PieceType src_piece_type = GetType(src_piece);

    const Color my_color = GetColor(src_piece);
    const Color their_color = FlipColor(my_color);

    const bool is_pawn = src_piece_type == PieceType::Pawn;
    const Piece their_pawn = BuildPiece(PieceType::Pawn, their_color);

    const auto [src_file, src_rank] = src_sq.FileRank();
    const auto [dst_file, dst_rank] = dst_sq.FileRank();

    const MoveType move_type = p_move.GetType();

    DEV_ASSERT_MSG(src_piece != Piece::Null, "No piece found on 'from' square");
    DEV_ASSERT_MSG(SideToMove() == my_color, "Trying to move a piece of the wrong color");

#if 0
    // check if the move will change the castling rights
    let castling_rights =
        castling_right_mask(pos.state.castling_rights, src_sq, dst_sq, src_piece, dst_piece);

    // check if the move will generate an en passant square
    let mut en_passant_sq: Option<Square> = None;
    if is_mover_pawn {
        let dy = dst_rank.diff(src_rank).abs();
        debug_assert!(dy <= 2, "Pawn move must be 1 or 2 squares");
        if dy == 2 {
            let enemy_pawns = pos.bitboards[enemy_pawn.as_usize()];
            if (PAWN_EN_PASSANT_MASKS[mover_color.as_usize()][dst_sq.as_usize()] & enemy_pawns)
                .any()
            {
                en_passant_sq = Some(Square::make(src_file, Rank((src_rank.0 + dst_rank.0) / 2)));
            }
        }
    }
#endif

    // -------------- Update Board Start --------------

    m_state.captured_piece = dst_piece;
    p_undo = m_state;  // save old state as undo state

    DEV_ASSERT(m_state.occupancies[SideToMove()].Test(src_sq));

    MovePiece(m_board[src_piece], src_sq, dst_sq);

    unused(their_pawn);
    unused(is_pawn);

    const bool captured_something = dst_piece != Piece::Null;
    if (captured_something) {
        m_board[dst_piece].Unset(dst_sq);
    }

    switch (move_type) {
        case MoveType::Castling: {
        } break;
        case MoveType::EnPassant: {
        } break;
        case MoveType::Promotion: {
        } break;
        default: {
        } break;
    }
#if 0
    // special move handling
    match move_type {
        MoveType::Castling => {
            // Castling move, we need to move the king and rook
            debug_assert!(src_piece_type == PieceType::KING, "Castling must be a king move");
            debug_assert!(dst_piece == Piece::NONE, "Castling must not capture any piece");

            // king already moved to the destination square, only need to move the rook
            let index = castling_type(src_piece, src_sq, dst_sq);
            debug_assert!(index != CastlingType::None, "Invalid castling move");
            // move rook position
            let (piece, src_sq, to_sq) = CASTLING_ROOK_SQUARES[index as usize];
            move_piece(&mut pos.bitboards[piece.as_usize()], src_sq, to_sq);
        }
        MoveType::Promotion => {
            debug_assert!(src_piece_type == PieceType::PAWN);
            let promotion = Piece::get_piece(mover_color, mv.get_promotion().unwrap());
            pos.bitboards[src_piece_idx].unset(dst_sq.as_u8()); // Remove the pawn from the board
            pos.bitboards[promotion.as_usize()].set(dst_sq.as_u8()); // Place the promoted piece on the board
        }
        MoveType::EnPassant => {
            debug_assert!(src_piece_type == PieceType::PAWN, "En passant must be a pawn move");
            let enemy_sq = Square::make(dst_file, src_rank);
            let enemy = Piece::get_piece(enemy_color, PieceType::PAWN);

            pos.bitboards[enemy.as_usize()].unset(enemy_sq.as_u8());
        }
        _ => {}
    }
#endif

    // -------------- Update Board End --------------
    m_side_to_move = FlipColor(m_side_to_move);

#if 0
    pos.state.castling_rights = castling_rights;
    pos.state.en_passant = en_passant_sq;
    if captured_something || is_mover_pawn {
        pos.state.halfmove_clock = 0; // reset halfmove clock if a piece was captured or a non-pawn moved
    } else {
        pos.state.halfmove_clock += 1; // increment halfmove clock for a pawn move
    }
#endif

    m_state.fullmove_number += my_color == Color::Black;  // increase after black moves

    return UpdateCache();
}

bool Position::UnmakeMove(Move p_move, UndoState& p_undo) {
    const Square src_sq = p_move.from;
    const Square dst_sq = p_move.to;

    const Piece src_piece = PieceAt(dst_sq);

    const Color my_color = GetColor(src_piece);
    const Color their_color = FlipColor(my_color);
    const Piece their_pawn = BuildPiece(PieceType::Pawn, their_color);

    MovePiece(m_board[src_piece], dst_sq, src_sq);

    const Piece captured_piece = p_undo.captured_piece;
    if (captured_piece != Piece::Null) {
        m_board[captured_piece].Set(dst_sq);
    }

    const MoveType move_type = p_move.GetType();
    switch (move_type) {
        case MoveType::Castling: {
        } break;
        case MoveType::EnPassant: {
            (void)their_pawn;
        } break;
        case MoveType::Promotion: {
        } break;
        default:
            break;
    }

    m_side_to_move = FlipColor(m_side_to_move);
    m_state = p_undo;
    return true;
}

Position Position::Default() {
    auto res = FromFen(kDefaultFen);
    DEV_ASSERT(res.has_value());

    Position pos = *res;
    return pos;
}

std::expected<Position, FenError> Position::FromFen(std::string_view p_fen) {
    std::string_view board;
    std::string_view stm;
    std::string_view castling;
    std::string_view ep;
    std::string_view half;
    std::string_view full;
    if (!SplitFen6(p_fen, board, stm, castling, ep, half, full)) {
        return std::unexpected(FenError::InvalidFieldCount);
    }

    Position pos{};
    if (!ParseBoard(board, pos.m_board)) {
        return std::unexpected(FenError::InvalidBoard);
    }

    pos.UpdateCache();
    return pos;
}

bool Position::UpdateCache() {
    m_state.occupancies[Color::White] =
        m_board[Piece::WP] |
        m_board[Piece::WN] |
        m_board[Piece::WB] |
        m_board[Piece::WR] |
        m_board[Piece::WK] |
        m_board[Piece::WQ];
    m_state.occupancies[Color::Black] =
        m_board[Piece::BP] |
        m_board[Piece::BN] |
        m_board[Piece::BB] |
        m_board[Piece::BR] |
        m_board[Piece::BK] |
        m_board[Piece::BQ];
    m_state.occupancies[Color::Both] =
        m_state.occupancies[Color::White] |
        m_state.occupancies[Color::Black];

    Color prev_color = FlipColor(m_side_to_move);

    const auto wk_bb = m_board[Piece::WK];
    const auto bk_bb = m_board[Piece::BK];
    m_state.king_squares[Color::White] = Square((uint8_t)std::countr_zero(wk_bb.Bits()));
    m_state.king_squares[Color::Black] = Square((uint8_t)std::countr_zero(bk_bb.Bits()));

    // NOTE: when white to move,
    // it cares about black pieces that check its king
    MoveGen::AttackMapAndCheckers(*this,
                                  Color::White,
                                  m_state.attack_mask[Color::White],
                                  m_state.checkers[Color::Black]);
    MoveGen::AttackMapAndCheckers(*this,
                                  Color::Black,
                                  m_state.attack_mask[Color::Black],
                                  m_state.checkers[Color::White]);

    if (m_state.checkers[prev_color].Count() > 0) {
        assert(0 && "check not resolved");
        return false;
    }

    return true;
}

std::string Position::Fen() const {
    assert(0 && "TODO");
    return "";
}

std::string Position::DebugBoardString() const {
    return chess::core::DebugBoardString(m_board);
}

static bool SplitFen6(std::string_view fen,
                      std::string_view& board,
                      std::string_view& stm,
                      std::string_view& castling,
                      std::string_view& ep,
                      std::string_view& half,
                      std::string_view& full) {
    std::string_view parts[6]{};
    int count = 0;

    // trim
    while (!fen.empty() && std::isspace(static_cast<unsigned char>(fen.front()))) fen.remove_prefix(1);
    while (!fen.empty() && std::isspace(static_cast<unsigned char>(fen.back()))) fen.remove_suffix(1);

    while (!fen.empty() && count < 6) {
        size_t i = 0;
        while (i < fen.size() && !std::isspace(static_cast<unsigned char>(fen[i]))) i++;
        parts[count++] = fen.substr(0, i);
        fen.remove_prefix(i);
        while (!fen.empty() && std::isspace(static_cast<unsigned char>(fen.front()))) fen.remove_prefix(1);
    }

    if (count != 6) return false;

    board = parts[0];
    stm = parts[1];
    castling = parts[2];
    ep = parts[3];
    half = parts[4];
    full = parts[5];
    return true;
}

static bool ParseBoard(std::string_view board,
                       cave::EnumArray<Piece, Bitboard, kPieceMax>& p_out) {
    uint8_t fen_rank = 8;  // 8 down to 1
    uint8_t file = 0;

    for (size_t i = 0; i < board.size(); ++i) {
        const char c = board[i];

        if (c == '/') {
            if (file != 8) return false;
            fen_rank--;
            if (fen_rank < 1) return false;
            file = 0;
            continue;
        }

        if (std::isdigit(c)) {
            const uint8_t empties = c - '0';
            if (empties < 1 || empties > 8) return false;
            file += empties;
            if (file > 8) return false;
            continue;
        }

        const Piece p = ParsePieceChar(c);
        if (p == Piece::Null) return false;
        if (file >= 8) return false;

        const uint8_t rank = fen_rank - 1;
        const Square sq = Square::FromFileRank(file, rank);

        // Set bit in the piece board.
        p_out[p].Set(sq);

        file++;
        if (file > 8) return false;
    }

    return (fen_rank == 1) && (file == 8);
}

static std::string DebugBoardString(const Position::Board& p_board) {
    std::string out;
    out.reserve(8 * (2 + 1 + 2 * 8) + 32);

    auto get_piece = [&p_board](Square sq) {
        for (uint8_t p = 0; p < kPieceMax; ++p) {
            const Piece piece = static_cast<Piece>(p);
            if (p_board[piece].Test(sq)) {
                return piece;
            }
        }
        return Piece::Null;
    };

    // ranks 8 -> 1
    for (uint8_t fen_rank = 8; fen_rank >= 1; --fen_rank) {
        const uint8_t rank = fen_rank - 1;

        out.push_back(char('0' + fen_rank));
        out.push_back(' ');
        out.push_back(' ');

        for (uint8_t file = 0; file < 8; ++file) {
            // If your Square is different, adapt this line:
            const Square sq = Square::FromFileRank(file, rank);

            const Piece piece = get_piece(sq);
            out.push_back(GetPieceChar(piece));

            if (file != 7) {
                out.push_back(' ');
                out.push_back(' ');
            }
        }

        out.push_back('\n');
    }

    // "   a b c d e f g h"
    out.append("   a  b  c  d  e  f  g  h\n");

    return out;
}

#if defined(CAVE_TEST)

TEST(SplitFen6, should_split_default_fen_correctly) {
    std::string_view board;
    std::string_view stm;
    std::string_view castling;
    std::string_view ep;
    std::string_view half;
    std::string_view full;

    ASSERT_TRUE(SplitFen6(kDefaultFen,
                          board,
                          stm,
                          castling,
                          ep,
                          half,
                          full));
    EXPECT_EQ(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    EXPECT_EQ(stm, "w");
    EXPECT_EQ(castling, "KQkq");
    EXPECT_EQ(ep, "-");
    EXPECT_EQ(half, "0");
    EXPECT_EQ(full, "1");
}

TEST(ParseBoard, should_parse_default_fen_correctly) {
    std::string_view board_str = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";

    Position::Board board;
    ASSERT_TRUE(ParseBoard(board_str, board));

    std::string pretty = DebugBoardString(board);
    constexpr std::string_view expect =
        "8  r  n  b  q  k  b  n  r\n"
        "7  p  p  p  p  p  p  p  p\n"
        "6  .  .  .  .  .  .  .  .\n"
        "5  .  .  .  .  .  .  .  .\n"
        "4  .  .  .  .  .  .  .  .\n"
        "3  .  .  .  .  .  .  .  .\n"
        "2  P  P  P  P  P  P  P  P\n"
        "1  R  N  B  Q  K  B  N  R\n"
        "   a  b  c  d  e  f  g  h\n";
    EXPECT_EQ(pretty, expect);
}

TEST(Position, piece_at) {
    Position pos = Position::Default();

    EXPECT_EQ(pos.PieceAt(Square(0)), Piece::WR);
    EXPECT_EQ(pos.PieceAt(Square(48)), Piece::BP);
    EXPECT_EQ(pos.PieceAt(Square(63)), Piece::BR);
}

#endif

}  // namespace chess::core