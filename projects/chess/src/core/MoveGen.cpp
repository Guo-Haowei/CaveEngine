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

static constexpr uint8_t MV_TYPE_MOVE = 0;
static constexpr uint8_t MV_TYPE_CAPTURE = 1;
static constexpr uint8_t MV_TYPE_ATTACK = 2;

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

    uint8_t sq = 0;
    while (sq < 64) {
        uint8_t file = sq % 8;
        uint8_t rank = sq / 8;

        masks[0][sq] = BuildPawnAttackMask<true>(file, rank);
        masks[1][sq] = BuildPawnAttackMask<false>(file, rank);

        sq += 1;
    }

    return masks;
}

static constexpr std::array<std::array<Bitboard, 64>, 2> kPawnAttackMasks = BuildPawnAttackMasks();

template<uint8_t COLOR, uint8_t MV_TYPE>
static Bitboard PawnMask(const Position& p_pos, Square p_sq) {
    constexpr uint8_t OPPONENT = COLOR ^ 1;

    constexpr bool is_white = COLOR == 0;

    const Bitboard attack_mask = kPawnAttackMasks[COLOR][p_sq.Index()];

    if constexpr (MV_TYPE == MV_TYPE_ATTACK) {
        return attack_mask;
    }

    const auto& occupancies = p_pos.State().occupancies;
    const Bitboard capture_mask = attack_mask & occupancies[static_cast<Color>(OPPONENT)];
    if constexpr (MV_TYPE == MV_TYPE_CAPTURE) {
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

template<uint8_t COLOR, uint8_t MV_TYPE>
static void PawnMoves(const Position& p_pos,
                          Square p_from_sq,
                          // @TODO: checker
                          // @TODO: king
                          MoveList& p_move_list) {
    const Bitboard pawn_mask = PawnMask<COLOR, MV_TYPE>(p_pos, p_from_sq);

    constexpr bool NOT_IN_CHECK = true;

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
            // if (NOT_IN_CHECK || ResolveCheck)
            {
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

void MoveGen::Pseudo(const Position& p_pos,
                     MoveList& p_move_list) {
    const Color stm = p_pos.SideToMove();

    for (uint8_t i = 0; i < kPieceTypeMax; ++i) {
        const Piece piece = BuildPiece(static_cast<PieceType>(i), stm);
        for (Square sq : p_pos.Bitboard(piece).Squares()) {
            PseudoFromSquare(p_pos, sq, piece, p_move_list);
        }
    }
}

void MoveGen::PseudoFromSquare(const Position& p_pos,
                               Square p_from,
                               Piece p_piece,
                               MoveList& p_move_list) {
    constexpr uint8_t MV_TYPE = MV_TYPE_MOVE;

    const Color color = p_pos.SideToMove();
    const Bitboard friendly = p_pos.m_state.occupancies[color];
    if (p_piece == Piece::WP) {
        PawnMoves<0 /* white */, MV_TYPE_MOVE>(p_pos, p_from, p_move_list);
        return;
    }
    if (p_piece == Piece::BP) {
        PawnMoves<1 /* black */, MV_TYPE_MOVE>(p_pos, p_from, p_move_list);
        return;
    }

    if (p_piece != Piece::Null) {
        return;
    }

    const auto [file, rank] = p_from.FileRank();
    Bitboard bb = MASK_FILES[file] | MASK_RANKS[rank];
    bb = bb & ~friendly;
    for (Square sq : bb.Squares()) {
        p_move_list.push_back({ p_from, sq });
    }
}

static_assert(kPawnAttackMasks[0][8] == Bitboard(1llu << 17));

}  // namespace chess::core
