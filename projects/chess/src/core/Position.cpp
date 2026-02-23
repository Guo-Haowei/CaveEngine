#include "Position.h"

#include <cassert>
#include "cave/core/typedefs.h"

#include "MoveGen.h"

namespace chess::core {

using cave::unused;

static constexpr const char kStartPosFen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

static bool SplitFen6(std::string_view fen,
                      std::string_view& board,
                      std::string_view& stm,
                      std::string_view& castling,
                      std::string_view& ep,
                      std::string_view& half,
                      std::string_view& full);

static bool ParseBoard(std::string_view board,
                       cave::EnumArray<Piece, Bitboard, kPieceMax>& p_out);

static bool ParseCastling(std::string_view p_str, CastlingRight& p_out);

static cave::Option<Square> ParseEnpassant(std::string_view p_str);

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

    assert(0 && "should not reach here");
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
    assert(p_board.Test(p_src) && "No piece found on 'src' square");
    p_board.Unset(p_src);
    p_board.Set(p_to);
}

static const std::tuple<Piece, Square, Square> kCastlingRookSquares[4] = {
    std::make_tuple(Piece::WR, Square::H1, Square::F1),  // White King-side
    std::make_tuple(Piece::WR, Square::A1, Square::D1),  // White Queen-side
    std::make_tuple(Piece::BR, Square::H8, Square::F8),  // Black King-side
    std::make_tuple(Piece::BR, Square::A8, Square::D8),  // Black Queen-side
};

enum class CastlingType : uint8_t {
    WhiteKingSide,
    WhiteQueenSide,
    BlackKingSide,
    BlackQueenSide,
    None,
};

// if castling rights are already disabled, return
// if king moved, disable castling rights, return
// if rook moved, disable castling rights, return
// if rook captured, disable castling rights, return
static CastlingRight UpdateCastling(CastlingRight p_old,
                               Square p_src_sq,
                               Square p_dst_sq,
                               Piece p_src_piece,
                               Piece p_dst_piece) {
    auto disable_castle_right = [](uint8_t p_bit,
                                   Square p_src_sq,
                                   Square p_dst_sq,
                                   Piece p_src_piece,
                                   Piece p_dst_piece) -> bool {
        CastlingRight mask = static_cast<CastlingRight>(1 << p_bit);
        const auto [rook, rook_sq, _] = kCastlingRookSquares[p_bit];

        // if the rook is moving away, the castling rights are disabled
        if (p_src_piece == rook && p_src_sq == rook_sq) {
            return true;
        }

        // if the rook was captured, the castling rights are disabled
        if (p_dst_piece == rook && p_dst_sq == rook_sq) {
            return true;
        }

        if (p_src_piece == Piece::WK && ((mask & CastlingRight::KQ) != CastlingRight::None)) {
            return true;
        }

        if (p_src_piece == Piece::BK && ((mask & CastlingRight::kq) != CastlingRight::None)) {
            return true;
        }

        return false;
    };

    CastlingRight new_mask = p_old;
    for (uint8_t i = 0; i < 4; ++i) {
        const CastlingRight flag = static_cast<CastlingRight>(1 << i);
        if ((new_mask & flag) == CastlingRight::None) {
            continue;  // if the castling right already disabled, continue
        }
        if (disable_castle_right(i, p_src_sq, p_dst_sq, p_src_piece, p_dst_piece)) {
            new_mask &= ~flag;
        }
    }
    return new_mask;
}

static CastlingType ConvertCastlingType(Piece src_piece, Square src_sq, Square dst_sq) {
    if (src_piece == Piece::WK && src_sq == Square::E1) {
        if (dst_sq == Square::G1) return CastlingType::WhiteKingSide;
        if (dst_sq == Square::C1) return CastlingType::WhiteQueenSide;
        return CastlingType::None;
    }
    if (src_piece == Piece::BK && src_sq == Square::E8) {
        if (dst_sq == Square::G8) return CastlingType::BlackKingSide;
        if (dst_sq == Square::C8) return CastlingType::BlackQueenSide;
        return CastlingType::None;
    }

    return CastlingType::None;
}

static constexpr Bitboard BuildEpMask(uint8_t file, uint8_t rank) {
    Bitboard mask(0);
    if (file > 0) {
        mask.Set(Square::FromFileRank(file - 1, rank));
    }
    if (file < 7) {
        mask.Set(Square::FromFileRank(file + 1, rank));
    }
    return mask;
}

static constexpr auto BuildEpMasks() -> std::array<std::array<Bitboard, 64>, 2> {
    std::array<std::array<Bitboard, 64>, 2> masks;
    for (uint8_t sq = 0; sq < 64; ++sq) {
        const uint8_t file = sq % 8;
        const uint8_t rank = sq / 8;
        if (rank == 3) {
            masks[0][sq] = BuildEpMask(file, rank);
        }
        if (rank == 4) {
            masks[1][sq] = BuildEpMask(file, rank);
        }
    }
    return masks;
}

static constexpr auto kEpMasks = BuildEpMasks();

bool Position::MakeMove(Move p_move, UndoState& p_undo) {
    const Square src_sq = p_move.From();
    const Square dst_sq = p_move.To();

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

    assert((src_piece != Piece::Null) && "No piece found on 'from' square");
    assert((SideToMove() == my_color) && "Trying to move a piece of the wrong color");

    // check if the move will change the castling rights
    const CastlingRight castling = UpdateCastling(m_state.castling, src_sq, dst_sq, src_piece, dst_piece);

    // check if the move will generate an en passant square
    cave::Option<Square> ep_sq = cave::None();
    if (is_pawn) {
        const int dy = std::abs((int)dst_rank - (int)src_rank);
        assert((dy <= 2) && "Pawn move must be 1 or 2 squares");
        if (dy == 2) {
            const auto enemy_pawn = m_board[their_pawn];
            if ((kEpMasks[std::to_underlying(my_color)][dst_sq.Index()] & enemy_pawn).Any()) {
                Square ep_sq_ = Square::FromFileRank(src_file, (src_rank + dst_rank) / 2);
                ep_sq = cave::Some(ep_sq_);
            }
        }
    }

    // -------------- Update Board Start --------------
    m_state.captured_piece = dst_piece;
    p_undo = m_state;  // save old state as undo state

    assert(m_state.occupancies[SideToMove()].Test(src_sq));

    MovePiece(m_board[src_piece], src_sq, dst_sq);

    const bool piece_captured = dst_piece != Piece::Null;
    if (piece_captured) {
        m_board[dst_piece].Unset(dst_sq);
    }

    switch (move_type) {
        case MoveType::Castling: {
            assert((src_piece_type == PieceType::King) && "Castling must be a king move");
            assert((dst_piece == Piece::Null) && "Castling must not capture any piece");

            // Castling move, we need to move the king and rook
            // king already moved to the destination square,
            // only need to move the rook
            const CastlingType castling_type = ConvertCastlingType(src_piece, src_sq, dst_sq);
            assert((castling_type != CastlingType::None) && "Invalid castling move");
            // move rook position
            const auto [rook, src_sq, rook_sq] = kCastlingRookSquares[std::to_underlying(castling_type)];
            MovePiece(m_board[rook], src_sq, rook_sq);
        } break;
        case MoveType::Enpassant: {
        } break;
        case MoveType::Promotion: {
            assert(src_piece_type == PieceType::Pawn);
            const Square enemy_sq = Square::FromFileRank(dst_file, src_rank);
            m_board[their_pawn].Unset(enemy_sq);
        } break;
        default: {
        } break;
    }
#if 0
    // special move handling
    match move_type {
        MoveType::Promotion => {
            debug_assert!(src_piece_type == PieceType::PAWN);
            let promotion = Piece::get_piece(mover_color, mv.get_promotion().unwrap());
            pos.bitboards[src_piece_idx].unset(dst_sq.as_u8()); // Remove the pawn from the board
            pos.bitboards[promotion.as_usize()].set(dst_sq.as_u8()); // Place the promoted piece on the board
        }
        _ => {}
    }
#endif

    // -------------- Update Board End --------------
    m_side_to_move = FlipColor(m_side_to_move);

    if (piece_captured || is_pawn) {
        m_state.halfmove_clock = 0;  // reset halfmove clock if a piece was captured or a non-pawn moved
    } else {
        m_state.halfmove_clock += 1;  // increment halfmove clock for a pawn move
    }

    m_state.castling = castling;
    m_state.ep = ep_sq;
    m_state.fullmove_number += my_color == Color::Black;  // increase after black moves

    return UpdateCache();
}

bool Position::UnmakeMove(Move p_move, UndoState& p_undo) {
    const Square src_sq = p_move.From();
    const Square dst_sq = p_move.To();

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
            assert(GetType(src_piece) == PieceType::King);

            // Restore Rook position
            CastlingType castling_type = ConvertCastlingType(src_piece, src_sq, dst_sq);
            assert(castling_type != CastlingType::None);

            const auto [rook, src_sq, rook_sq] = kCastlingRookSquares[std::to_underlying(castling_type)];
            MovePiece(m_board[rook], rook_sq, src_sq);
        } break;
        case MoveType::Enpassant: {
            const auto [_from_file, from_rank] = src_sq.FileRank();
            const auto [to_file, _to_rank] = dst_sq.FileRank();
            const Square enemy_sq = Square::FromFileRank(to_file, from_rank);
            m_board[their_pawn].Set(enemy_sq);
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

Position Position::Startpos() {
    auto res = FromFen(kStartPosFen);
    assert(res.has_value());

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

    if (stm == "w") {
        pos.m_side_to_move = Color::White;
    } else if (stm == "b") {
        pos.m_side_to_move = Color::Black;
    } else {
        return std::unexpected(FenError::InvalidSideToMove);
    }

    if (!ParseCastling(castling, pos.m_state.castling)) {
        return std::unexpected(FenError::InvalidCastling);
    }

    pos.m_state.ep = ParseEnpassant(ep);

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
        return false;
    }

    // update king squares after AttackMapAndCheckers
    const auto wk_bb = m_board[Piece::WK];
    const auto bk_bb = m_board[Piece::BK];
    m_state.king_squares[Color::White] = Square((uint8_t)std::countr_zero(wk_bb.Bits()));
    m_state.king_squares[Color::Black] = Square((uint8_t)std::countr_zero(bk_bb.Bits()));

    return true;
}

std::string Position::Fen() const {
    assert(0 && "TODO");
    return "";
}

std::string Position::DebugBoardString() const {
    return chess::core::DebugBoardString(m_board);
}

#pragma region FEN_PARSING
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

static bool ParseBoard(std::string_view p_str,
                       cave::EnumArray<Piece, Bitboard, kPieceMax>& p_out) {
    uint8_t fen_rank = 8;  // 8 down to 1
    uint8_t file = 0;

    for (size_t i = 0; i < p_str.size(); ++i) {
        const char c = p_str[i];

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

static bool ParseCastling(std::string_view p_str, CastlingRight& p_out) {
    p_out = CastlingRight::None;
    if (p_str == "-") return true;
    if (p_str.empty()) return false;

    for (char c : p_str) {
        switch (c) {
            case 'K': {
                p_out |= CastlingRight::K;
            } break;
            case 'Q': {
                p_out |= CastlingRight::Q;
            } break;
            case 'k': {
                p_out |= CastlingRight::k;
            } break;
            case 'q': {
                p_out |= CastlingRight::q;
            } break;
            default:
                return false;
        }
    }
    return true;
}

static cave::Option<Square> ParseEnpassant(std::string_view p_str) {
    if (p_str == "-" || p_str.size() != 2) return cave::None();

    const char f = p_str[0];
    const char r = p_str[1];

    if (f < 'a' || f > 'h' || r < '1' || r > '8')
        return cave::None();

    const int8_t file = f - 'a';
    const int8_t rank = r - '1';
    return cave::Some(Square::FromFileRank(file, rank));
}

#pragma endregion FEN_PARSING

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

    ASSERT_TRUE(SplitFen6(kStartPosFen,
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
    Position pos = Position::Startpos();

    EXPECT_EQ(pos.PieceAt(Square(0)), Piece::WR);
    EXPECT_EQ(pos.PieceAt(Square(48)), Piece::BP);
    EXPECT_EQ(pos.PieceAt(Square(63)), Piece::BR);
}

#endif

}  // namespace chess::core