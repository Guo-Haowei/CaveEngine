#include "Position.h"

namespace chess::core {

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

void Position::UpdateCache() {
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
    (void)prev_color;
#if 0
pub fn update_cache(pos: &mut Position) -> bool {

    let prev_color = pos.side_to_move.flip();

    // update attack maps
    let (white_attack_map, white_checkers) = move_gen::calc_attack_map_and_checker::<0>(pos);
    let (black_attack_map, black_checkers) = move_gen::calc_attack_map_and_checker::<1>(pos);

    pos.state.attack_mask[Color::WHITE.as_usize()] = white_attack_map;
    pos.state.attack_mask[Color::BLACK.as_usize()] = black_attack_map;
    // note that for black to move, it needs to check if there are any white checkers
    pos.state.checkers[Color::WHITE.as_usize()] = black_checkers;
    pos.state.checkers[Color::BLACK.as_usize()] = white_checkers;

    // the previous player didn't resolve the check, so the move was illegal
    if pos.state.checkers[prev_color.as_usize()].count() > 0 {
        return false;
    }

    // update the king squares
    let white_king_mask = pos.bitboards[Piece::W_KING.as_usize()].get();
    let black_king_mask = pos.bitboards[Piece::B_KING.as_usize()].get();
    debug_assert!(white_king_mask.count_ones() == 1);
    debug_assert!(black_king_mask.count_ones() == 1);
    pos.state.king_squares[0] = Square::new(white_king_mask.trailing_zeros() as u8);
    pos.state.king_squares[1] = Square::new(black_king_mask.trailing_zeros() as u8);

    true
}
#endif
}
std::string Position::Fen() const {
    return "TODO";
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