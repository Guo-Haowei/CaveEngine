#include "MoveGen.h"

#include "Position.h"

namespace chess::core {

static constexpr Bitboard MASK_A{ 0x0101010101010101 };
static constexpr Bitboard MASK_B{ 0x0202020202020202 };
static constexpr Bitboard MASK_C{ 0x0404040404040404 };
static constexpr Bitboard MASK_D{ 0x0808080808080808 };
static constexpr Bitboard MASK_E{ 0x1010101010101010 };
static constexpr Bitboard MASK_F{ 0x2020202020202020 };
static constexpr Bitboard MASK_G{ 0x4040404040404040 };
static constexpr Bitboard MASK_H{ 0x8080808080808080 };

static constexpr Bitboard MASK_1{ 0x00000000000000FF };
static constexpr Bitboard MASK_2{ 0x000000000000FF00 };
static constexpr Bitboard MASK_3{ 0x0000000000FF0000 };
static constexpr Bitboard MASK_4{ 0x00000000FF000000 };
static constexpr Bitboard MASK_5{ 0x000000FF00000000 };
static constexpr Bitboard MASK_6{ 0x0000FF0000000000 };
static constexpr Bitboard MASK_7{ 0x00FF000000000000 };
static constexpr Bitboard MASK_8{ 0xFF00000000000000 };

static constexpr std::array<Bitboard, 8> MASK_FILES = {
    MASK_A,
    MASK_B,
    MASK_C,
    MASK_D,
    MASK_E,
    MASK_F,
    MASK_G,
    MASK_H,
};

static constexpr std::array<Bitboard, 8> MASK_RANKS = {
    MASK_1,
    MASK_2,
    MASK_3,
    MASK_4,
    MASK_5,
    MASK_6,
    MASK_7,
    MASK_8,
};

enum class MoveMaskType {
    Move = 0,     // pseudo legal move
    Capture = 1,  // capture moves
    Attack = 2,   // tiles the piece protect
};

template<bool IS_WHITE>
static constexpr Bitboard BuildPawnAttackMask(uint8_t file, uint8_t rank) {
    Bitboard mask(0);

    if (rank == 0 || rank == 7) {
        return mask;  // No pawn moves on the first or last rank
    }
    if constexpr (IS_WHITE) {
        if (file > 0) {
            mask.Set(Square::FromFileRank(file - 1, rank + 1));
        }
        if (file < 7) {
            mask.Set(Square::FromFileRank(file + 1, rank + 1));
        }
    } else {
        if (file > 0) {
            mask.Set(Square::FromFileRank(file - 1, rank - 1));
        }
        if (file < 7) {
            mask.Set(Square::FromFileRank(file + 1, rank - 1));
        }
    }
    return mask;
}

static constexpr auto BuildPawnAttackMasks() -> std::array<std::array<Bitboard, 64>, 2> {
    std::array<std::array<Bitboard, 64>, 2> masks;

    for (uint8_t sq = 0; sq < 64; ++sq) {
        const uint8_t file = sq % 8;
        const uint8_t rank = sq / 8;

        masks[0][sq] = BuildPawnAttackMask<true>(file, rank);
        masks[1][sq] = BuildPawnAttackMask<false>(file, rank);
    }

    return masks;
}

static constexpr Bitboard BuildKnightMask(uint8_t file, uint8_t rank) {
    Bitboard mask(0);

    constexpr std::array<std::pair<int8_t, int8_t>, 8> OFFSETS = {
        std::make_pair(2, 1),
        std::make_pair(2, -1),
        std::make_pair(-2, 1),
        std::make_pair(-2, -1),
        std::make_pair(1, 2),
        std::make_pair(1, -2),
        std::make_pair(-1, 2),
        std::make_pair(-1, -2),
    };

    for (size_t idx = 0; idx < OFFSETS.size(); ++idx) {
        const auto [df, dr] = OFFSETS[idx];
        const int8_t new_file = file + df;
        const int8_t new_rank = rank + dr;
        if (new_file >= 0 && new_file < 8 && new_rank >= 0 && new_rank < 8) {
            mask.Set(Square::FromFileRank(new_file, new_rank));
        }
    }
    return mask;
}

static constexpr auto BuildKnightMasks() -> std::array<Bitboard, 64> {
    std::array<Bitboard, 64> masks;

    for (uint8_t sq = 0; sq < 64; ++sq) {
        const uint8_t file = sq % 8;
        const uint8_t rank = sq / 8;
        masks[sq] = BuildKnightMask(file, rank);
    }

    return masks;
}

static constexpr std::array<std::array<Bitboard, 64>, 2> kPawnAttackMasks = BuildPawnAttackMasks();
static constexpr std::array<Bitboard, 64> kKnightMasks = BuildKnightMasks();

template<uint8_t COLOR, MoveMaskType MV_TYPE>
static Bitboard PawnMask(const Position& p_pos, Square p_sq) {
    constexpr uint8_t OPPONENT = COLOR ^ 1;

    constexpr bool is_white = COLOR == 0;

    const Bitboard attack_mask = kPawnAttackMasks[COLOR][p_sq.Index()];

    if constexpr (MV_TYPE == MoveMaskType::Attack) {
        return attack_mask;
    }

    const auto& occupancies = p_pos.State().occupancies;
    const Bitboard capture_mask = attack_mask & occupancies[static_cast<Color>(OPPONENT)];
    if constexpr (MV_TYPE == MoveMaskType::Capture) {
        return capture_mask;
    }

    constexpr int8_t offset = is_white ? 8 : -8;
    const int8_t advance_once_bit = (int8_t)p_sq.Index() + offset;

    const Bitboard empty_mask = ~occupancies[Color::Both];
    Bitboard advance_once(0);
    if (advance_once_bit >= 0 && advance_once_bit < 64) {
        advance_once.Set(Square(advance_once_bit));
        advance_once &= empty_mask;
    }

    Bitboard advance_twice(0);
    if (advance_once.Any()) {
        const int8_t advance_twice_bit = (int8_t)p_sq.Index() + 2 * offset;
        if (advance_twice_bit >= 0 && advance_twice_bit < 64) {
            advance_twice.Set(Square(advance_twice_bit));
            advance_twice &= empty_mask;
            advance_twice &= (is_white ? MASK_4 : MASK_5);
        }
    }

    return advance_once | advance_twice | capture_mask;
}

template<uint8_t COLOR, MoveMaskType MV_TYPE>
static void PawnMoves(const Position& p_pos,
                      Square p_from_sq,
                      // @TODO: checker
                      // @TODO: king
                      MoveList& p_move_list) {
    constexpr bool NOT_IN_CHECK = true;

    const Bitboard pawn_mask = PawnMask<COLOR, MV_TYPE>(p_pos, p_from_sq);

    for (Square to_sq : pawn_mask.Squares()) {
        // @TODO: promotion
#if 0
        let sq_mask = 1u64 << dst_sq.as_u8();
        let promo_rank = if COLOR == 0 { BitBoard::MASK_8 } else { BitBoard::MASK_1 };
        if sq_mask & promo_rank != 0 {
            // Promotion move
            let promotion_types =
                [PieceType::QUEEN, PieceType::ROOK, PieceType::BISHOP, PieceType::KNIGHT];
            for &promotion in &promotion_types {
                if NOT_IN_CHECK || resolve_check(dst_sq, checker_sq, king_sq) {
                    move_list.add(Move::new(sq, dst_sq, MoveType::Promotion, Some(promotion)));
                }
            }
        } else
#endif
        {
            if (NOT_IN_CHECK /* || ResolveCheck */) {
                p_move_list.push_back(Move{ p_from_sq, to_sq });
            }
        }
    }

    // @TODO: en passant
#if 0
    if let Some(ep_sq) = pos.state.en_passant {
        let attack_mask = pawn_mask::<{ COLOR }, MV_MASK_ATTACK>(sq, pos);
        // if attach mask and ep square overlap, then it's an en passant capture
        if attack_mask.test_sq(ep_sq) {
            // if we can make an en passant capture, it means the enemy just moved the pawn,
            // and the pawn formed capture,
            // so if we can take out the pawn just pushed, then it's a legal move
            if NOT_IN_CHECK || checker_type == PieceType::PAWN {
                move_list.add(Move::new(sq, ep_sq, MoveType::EnPassant, None));
            }
        }
    }
#endif
}

template<MoveMaskType MV_TYPE>
static Bitboard KnightMask(Square p_from_sq,
                           Bitboard p_friendly,
                           Bitboard p_enemy) {
    const Bitboard mask = kKnightMasks[p_from_sq.Index()];

    switch (MV_TYPE) {
        case MoveMaskType::Move:
            return mask & (~p_friendly);
        case MoveMaskType::Capture:
            return mask & p_enemy;
        case MoveMaskType::Attack:
        default:
            return mask;
    }
}

static Bitboard GenRay(Square p_from_sq,
                       Bitboard p_friendly,
                       Bitboard p_enemy,
                       int8_t p_dx,
                       int8_t p_dy) {
    Bitboard mask(0);

    auto [file, rank] = p_from_sq.FileRank();
    for (;;) {
        file += p_dx;
        rank += p_dy;
        if (file >= 8 || rank >= 8) break;
        const Square sq = Square::FromFileRank(file, rank);
        if (p_friendly.Test(sq)) break;
        mask.Set(sq);
        if (p_enemy.Test(sq)) break;
    } 

    return mask;
}

template<MoveMaskType MV_TYPE>
static Bitboard BishopMask(Square p_from_sq,
                           Bitboard p_friendly,
                           Bitboard p_enemy) {
    Bitboard mask = GenRay(p_from_sq, p_friendly, p_enemy, -1, -1);
    mask |= GenRay(p_from_sq, p_friendly, p_enemy, -1, 1);
    mask |= GenRay(p_from_sq, p_friendly, p_enemy, 1, -1);
    mask |= GenRay(p_from_sq, p_friendly, p_enemy, 1, 1);

    switch (MV_TYPE) {
        case MoveMaskType::Move:
            return mask & (~p_friendly);
        case MoveMaskType::Capture:
            return mask & p_enemy;
        case MoveMaskType::Attack:
        default:
            return mask;
    }
}

template<MoveMaskType MV_TYPE>
static Bitboard RookMask(Square p_from_sq,
                         Bitboard p_friendly,
                         Bitboard p_enemy) {
    Bitboard mask = GenRay(p_from_sq, p_friendly, p_enemy, 0, -1);
    mask |= GenRay(p_from_sq, p_friendly, p_enemy, 0, 1);
    mask |= GenRay(p_from_sq, p_friendly, p_enemy, -1, 0);
    mask |= GenRay(p_from_sq, p_friendly, p_enemy, 1, 0);

    switch (MV_TYPE) {
        case MoveMaskType::Move:
            return mask & (~p_friendly);
        case MoveMaskType::Capture:
            return mask & p_enemy;
        case MoveMaskType::Attack:
        default:
            return mask;
    }
}

template<MoveMaskType MV_TYPE>
static Bitboard QueenMask(Square p_from_sq,
                          Bitboard p_friendly,
                          Bitboard p_enemy) {
    Bitboard mask = GenRay(p_from_sq, p_friendly, p_enemy, 0, -1);
    mask |= GenRay(p_from_sq, p_friendly, p_enemy, 0, 1);
    mask |= GenRay(p_from_sq, p_friendly, p_enemy, -1, 0);
    mask |= GenRay(p_from_sq, p_friendly, p_enemy, 1, 0);
    mask |= GenRay(p_from_sq, p_friendly, p_enemy, -1, -1);
    mask |= GenRay(p_from_sq, p_friendly, p_enemy, -1, 1);
    mask |= GenRay(p_from_sq, p_friendly, p_enemy, 1, -1);
    mask |= GenRay(p_from_sq, p_friendly, p_enemy, 1, 1);

    switch (MV_TYPE) {
        case MoveMaskType::Move:
            return mask & (~p_friendly);
        case MoveMaskType::Capture:
            return mask & p_enemy;
        case MoveMaskType::Attack:
        default:
            return mask;
    }
}

#if 0
fn pseudo_legal_move_king<const COLOR: u8, const MASK_TYPE: u8>(
    move_list: &mut MoveList,
    sq: Square,
    pos: &Position,
) {
    let mask = king_mask::<COLOR, MASK_TYPE>(sq, pos);
    for dst_sq in mask.iter() {
        // check if it's a castling move
        let (src_file, _) = sq.file_rank();
        let (dst_file, _) = dst_sq.file_rank();
        let diff = src_file.diff(dst_file);
        let move_type = match diff.abs() {
            0 | 1 => MoveType::Normal,
            2 => MoveType::Castling,
            _ => panic!("Invalid castling move from {} to {}", sq, dst_sq),
        };

        move_list.add(Move::new(sq, dst_sq, move_type, None));
    }
}

#endif

MoveList MoveGen::Pseudo(const Position& p_pos) {
    MoveList moves;
    moves.reserve(128);

    const Color stm = p_pos.SideToMove();
    const Square king_sq = p_pos.GetKing(stm);


    #if 0
    let color = pos.side_to_move;
    let king_sq = pos.get_king_square(color);
    let (start, end) = if color == Color::WHITE {
        pseudo_legal_move_king::<0, MASK>(&mut move_list, king_sq, pos);
        (Piece::W_START, Piece::W_END)
    } else {
        pseudo_legal_move_king::<1, MASK>(&mut move_list, king_sq, pos);
        (Piece::B_START, Piece::B_END)
    };

    // early return if double check
    let checkers = &pos.state.checkers[color.as_usize()];
    if checkers.count() == 2 {
        return move_list;
    }

    #endif

    for (uint8_t i = 0; i < kPieceTypeMax; ++i) {
        const Piece piece = BuildPiece(static_cast<PieceType>(i), stm);
        for (Square sq : p_pos.Bitboard(piece).Squares()) {
            PseudoFromSquare(p_pos, sq, piece, moves);
        }
    }
    return moves;
}

void MoveGen::PseudoFromSquare(const Position& p_pos,
                               Square p_from,
                               Piece p_piece,
                               MoveList& p_move_list) {
    constexpr MoveMaskType MV_TYPE = MoveMaskType::Move;

    const Color color = p_pos.SideToMove();
    const Bitboard friendly = p_pos.m_state.occupancies[color];
    const Bitboard enemy = p_pos.m_state.occupancies[FlipColor(color)];
    if (p_piece == Piece::WP) {
        PawnMoves<0 /* white */, MV_TYPE>(p_pos, p_from, p_move_list);
        return;
    }
    if (p_piece == Piece::BP) {
        PawnMoves<1 /* black */, MV_TYPE>(p_pos, p_from, p_move_list);
        return;
    }

    const PieceType piece_type = GetType(p_piece);
    Bitboard mask(0);
    switch (piece_type) {
        case PieceType::Knight: {
            mask = KnightMask<MV_TYPE>(p_from, friendly, enemy);
        } break;
        case PieceType::Bishop: {
            mask = BishopMask<MV_TYPE>(p_from, friendly, enemy);
        } break;
        case PieceType::Rook: {
            mask = RookMask<MV_TYPE>(p_from, friendly, enemy);
        } break;
        case PieceType::Queen: {
            mask = QueenMask<MV_TYPE>(p_from, friendly, enemy);
        } break;
        default: {
        } break;
    }

    for (Square sq : mask.Squares()) {
        // if not in check or move resolve check
        p_move_list.push_back({ p_from, sq });
    }
}

static_assert(kPawnAttackMasks[0][8] == Bitboard(1llu << 17));

#if defined(CAVE_TEST)

static uint64_t Perft(Position& p_pos, uint8_t p_depth) {
    if (p_depth <= 0) return 1;

    std::vector<Move> moves;
    moves.reserve(128);
    MoveGen::Pseudo(p_pos, moves);

    uint64_t nodes = 0;
    for (Move mv : moves) {
        UndoState undo{};

        p_pos.MakeMove(mv, undo);
        nodes += Perft(p_pos, p_depth - 1);
        p_pos.UnmakeMove(mv, undo);
    }

    return nodes;
}

static std::vector<std::pair<std::string, uint64_t>> PerftDivide(Position& pos, uint8_t depth) {
    std::vector<std::pair<std::string, uint64_t>> out;

    std::vector<Move> moves;
    moves.reserve(128);
    MoveGen::Pseudo(pos, moves);

    out.reserve(moves.size());

    for (const Move& mv : moves) {
        UndoState undo{};
        pos.MakeMove(mv, undo);
        const uint64_t n = (depth <= 1) ? 1ULL : Perft(pos, depth - 1);
        pos.UnmakeMove(mv, undo);
        out.emplace_back(mv.Uci(), n);
    }
    return out;
}

static void PerftTestHelper(const char* p_fen,
                            uint8_t p_depth,
                            std::span<uint64_t> p_expect) {
    Position pos = *Position::FromFen(p_fen);

    printf("testing position: '%s'\n", p_fen);
    for (uint8_t i = 0; i <= p_depth; ++i) {
        const uint64_t nodes = Perft(pos, i);
        printf("depth %d: %llu nodes\n", i, nodes);

        if (nodes != p_expect[i]) {
            // Do a divide to localize the bug:
            Position pos2 = *Position::FromFen(p_fen);
            auto div = PerftDivide(pos2, i);

            printf("Divide:\n");
            for (auto& [uci, n] : div) {
                printf("  %s : %llu\n", uci.c_str(), n);
            }
        }

        EXPECT_EQ(nodes, p_expect[i]);
    }
}

TEST(MoveGen, perft_test_initial_position) {
    std::array<uint64_t, 8> tests{
        1,
        20,
        400,
        8902,
        197281,
        4865609,
        119060324,
        3195901860,  // depth 7
    };

    // @TODO: implement king move
    constexpr uint8_t depth = 3;

    PerftTestHelper("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                    depth,
                    tests);

}
#endif

}  // namespace chess::core
