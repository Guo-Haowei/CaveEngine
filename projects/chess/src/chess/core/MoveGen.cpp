#include "MoveGen.h"

#include <cassert>

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
    Move = 0,    // pseudo legal move
    Attack = 1,  // tiles the piece protect
};

#pragma region COMPILE_TIME_MASK
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

    constexpr std::array<std::pair<int8_t, int8_t>, 8> offsets = {
        std::make_pair<int8_t, int8_t>(+2, +1),
        std::make_pair<int8_t, int8_t>(+2, -1),
        std::make_pair<int8_t, int8_t>(-2, +1),
        std::make_pair<int8_t, int8_t>(-2, -1),
        std::make_pair<int8_t, int8_t>(+1, +2),
        std::make_pair<int8_t, int8_t>(+1, -2),
        std::make_pair<int8_t, int8_t>(-1, +2),
        std::make_pair<int8_t, int8_t>(-1, -2),
    };

    for (size_t idx = 0; idx < offsets.size(); ++idx) {
        const auto [df, dr] = offsets[idx];
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

static constexpr Bitboard BuildKingMask(uint8_t file, uint8_t rank) {
    Bitboard mask(0);

    constexpr std::array<std::pair<int8_t, int8_t>, 8> offsets = {
        std::make_pair<int8_t, int8_t>(+0, +1),
        std::make_pair<int8_t, int8_t>(+0, -1),
        std::make_pair<int8_t, int8_t>(-1, +0),
        std::make_pair<int8_t, int8_t>(+1, +0),

        std::make_pair<int8_t, int8_t>(-1, -1),
        std::make_pair<int8_t, int8_t>(-1, +1),
        std::make_pair<int8_t, int8_t>(+1, -1),
        std::make_pair<int8_t, int8_t>(+1, +1),
    };

    for (size_t idx = 0; idx < offsets.size(); ++idx) {
        const auto [df, dr] = offsets[idx];
        const int8_t new_file = file + df;
        const int8_t new_rank = rank + dr;
        if (new_file >= 0 && new_file < 8 && new_rank >= 0 && new_rank < 8) {
            mask.Set(Square::FromFileRank(new_file, new_rank));
        }
    }
    return mask;
}

static constexpr auto BuildKingMasks() -> std::array<Bitboard, 64> {
    std::array<Bitboard, 64> masks;

    for (uint8_t sq = 0; sq < 64; ++sq) {
        const uint8_t file = sq % 8;
        const uint8_t rank = sq / 8;
        masks[sq] = BuildKingMask(file, rank);
    }

    return masks;
}

static constexpr std::array<std::array<Bitboard, 64>, 2> kPawnAttackMasks = BuildPawnAttackMasks();
static constexpr std::array<Bitboard, 64> kKnightMasks = BuildKnightMasks();
static constexpr std::array<Bitboard, 64> kKingMasks = BuildKingMasks();
#pragma endregion COMPILE_TIME_MASK

static bool ResolveCheck(Square p_dst_sq, Square p_checker_sq, Square p_king_sq) {
    // 1) eliminate checker
    if (p_dst_sq == p_checker_sq) {
        return true;
    }
    // 2) block attacker
    if (p_dst_sq.SameLineInclusive(p_checker_sq, p_king_sq)) {
        return true;
    }

    return false;
}

template<MoveMaskType MV_TYPE>
static Bitboard PawnMask(Color p_color, const Position& p_pos, Square p_sq) {
    const uint8_t OPPONENT = std::to_underlying(p_color) ^ 1;

    const bool is_white = p_color == Color::White;

    const Bitboard attack_mask = kPawnAttackMasks[std::to_underlying(p_color)][p_sq.Index()];

    // @NOTE: assign to variable to avoid compiler warning
    if (MoveMaskType type = MV_TYPE; type == MoveMaskType::Attack) {
        return attack_mask;
    }

    const auto& occupancies = p_pos.State().occupancies;
    const Bitboard capture_mask = attack_mask & occupancies[static_cast<Color>(OPPONENT)];

    const int8_t offset = is_white ? 8 : -8;
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

template<MoveMaskType MV_TYPE>
static void PawnMoves(Color p_color,
                      const Position& p_pos,
                      Square p_from_sq,
                      MoveList& p_move_list,
                      Square p_king_sq,
                      bool p_in_check,
                      Square p_checker_sq,
                      PieceType p_checker_type) {
    const Bitboard pawn_mask = PawnMask<MV_TYPE>(p_color, p_pos, p_from_sq);

    const Bitboard promo_rank = p_color == Color::White ? MASK_8 : MASK_1;
    for (Square to_sq : pawn_mask.Squares()) {
        if (promo_rank.Test(to_sq)) {
            static constexpr std::array<PieceType, 4> kPromoTypes = {
                PieceType::Queen,
                PieceType::Rook,
                PieceType::Bishop,
                PieceType::Knight,
            };
            for (PieceType piece_type : kPromoTypes) {
                if (!p_in_check || ResolveCheck(to_sq, p_checker_sq, p_king_sq)) {
                    p_move_list.Add(Move::Promotion(p_from_sq, to_sq, piece_type));
                }
            }
        } else {
            if (!p_in_check || ResolveCheck(to_sq, p_checker_sq, p_king_sq)) {
                p_move_list.Add(Move::Normal(p_from_sq, to_sq));
            }
        }
    }

    if (p_pos.State().ep.is_some()) {
        const Square ep_sq = p_pos.State().ep.unwrap_unchecked();
        const Bitboard attack_mask = PawnMask<MoveMaskType::Attack>(p_color, p_pos, p_from_sq);
        if (attack_mask.Test(ep_sq)) {
            // if we can make an en passant capture,
            // it means the enemy just moved the pawn.
            // and the pawn is capture the king,
            // so if we can take eliminate the pawn just pushed
            // then capture is resolved
            if (!p_in_check || p_checker_type == PieceType::Pawn) {
                p_move_list.Add(Move::Enpassant(p_from_sq, ep_sq));
            }
        }
    }
}

template<MoveMaskType MV_TYPE>
static Bitboard KnightMask(Square p_from_sq,
                           Bitboard p_friendly) {
    const Bitboard mask = kKnightMasks[p_from_sq.Index()];

    return (MV_TYPE == MoveMaskType::Move) ? (mask & (~p_friendly)) : mask;
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

    return (MV_TYPE == MoveMaskType::Move) ? (mask & (~p_friendly)) : mask;
}

template<MoveMaskType MV_TYPE>
static Bitboard RookMask(Square p_from_sq,
                         Bitboard p_friendly,
                         Bitboard p_enemy) {
    Bitboard mask = GenRay(p_from_sq, p_friendly, p_enemy, 0, -1);
    mask |= GenRay(p_from_sq, p_friendly, p_enemy, 0, 1);
    mask |= GenRay(p_from_sq, p_friendly, p_enemy, -1, 0);
    mask |= GenRay(p_from_sq, p_friendly, p_enemy, 1, 0);

    return (MV_TYPE == MoveMaskType::Move) ? (mask & (~p_friendly)) : mask;
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

    return (MV_TYPE == MoveMaskType::Move) ? (mask & (~p_friendly)) : mask;
}

static const uint64_t B1_MASK = 1llu << Square::B1.Index();
static const uint64_t C1_MASK = 1llu << Square::C1.Index();
static const uint64_t D1_MASK = 1llu << Square::D1.Index();
static const uint64_t E1_MASK = 1llu << Square::E1.Index();
static const uint64_t F1_MASK = 1llu << Square::F1.Index();
static const uint64_t G1_MASK = 1llu << Square::G1.Index();

static const uint64_t B8_MASK = 1llu << Square::B8.Index();
static const uint64_t C8_MASK = 1llu << Square::C8.Index();
static const uint64_t D8_MASK = 1llu << Square::D8.Index();
static const uint64_t E8_MASK = 1llu << Square::E8.Index();
static const uint64_t F8_MASK = 1llu << Square::F8.Index();
static const uint64_t G8_MASK = 1llu << Square::G8.Index();

static const Bitboard kCastlingClearMasks[4] = {
    Bitboard(F1_MASK | G1_MASK),            // White kingside
    Bitboard(B1_MASK | C1_MASK | D1_MASK),  // White queenside
    Bitboard(F8_MASK | G8_MASK),            // Black kingside
    Bitboard(B8_MASK | C8_MASK | D8_MASK),  // Black queenside
};

static const Bitboard kCastlingSafeMasks[4] = {
    Bitboard(E1_MASK | F1_MASK | G1_MASK),  // White kingside
    Bitboard(C1_MASK | D1_MASK | E1_MASK),  // White queenside
    Bitboard(E8_MASK | F8_MASK | G8_MASK),  // Black kingside
    Bitboard(C8_MASK | D8_MASK | E8_MASK),  // Black queenside
};

static const Square kCastlingKingDests[4] = {
    Square::G1,  // White kingside
    Square::C1,  // White queenside
    Square::G8,  // Black kingside
    Square::C8,  // Black queenside
};

static const Square kCastlingRookDests[4] = {
    Square::H1,  // White kingside
    Square::A1,  // White queenside
    Square::H8,  // Black kingside
    Square::A8,  // Black queenside
};

template<MoveMaskType MV_TYPE>
static Bitboard KingMask(Color p_color,
                         Square p_square,
                         const Position& p_pos) {

    Bitboard mask = kKingMasks[p_square.Index()];

    if constexpr (MV_TYPE == MoveMaskType::Move) {
        // exclude friendly pieces
        mask &= ~p_pos.State().occupancies[p_color];
        // exclude pieces that are under attack
        mask &= ~p_pos.State().attack_mask[FlipColor(p_color)];

        // castling
        const uint8_t offset = p_color == Color::White ? 0 : 2;
        for (uint8_t i = 0; i < 2; ++i) {
            const uint8_t bit = i + offset;
            const CastlingRight flag = static_cast<CastlingRight>(1 << bit);
            if ((flag & p_pos.State().castling) == CastlingRight::None) {
                continue;
            }

            const bool path_clear = (kCastlingClearMasks[bit] & p_pos.State().occupancies[Color::Both]).Empty();
            const bool path_safe = (kCastlingSafeMasks[bit] & p_pos.State().attack_mask[FlipColor(p_color)]).Empty();
            const bool rook_not_moved = p_pos.Bitboard(BuildPiece(PieceType::Rook, p_color)).Test(kCastlingRookDests[bit]);

            if (path_clear && path_safe && rook_not_moved) {
                mask.Set(kCastlingKingDests[bit]);
            }
        }
    }

    return mask;
}

template<MoveMaskType MV_TYPE>
static void KingMoves(Color p_color,
                      Square p_src_sq,
                      const Position& p_pos,
                      MoveList& p_move_list) {
    const Bitboard mask = KingMask<MV_TYPE>(p_color, p_src_sq, p_pos);
    for (Square dst_sq : mask.Squares()) {
        const auto [src_file, src_rank] = p_src_sq.FileRank();
        const auto [dst_file, dst_rank] = dst_sq.FileRank();
        int8_t diff = (int8_t)src_file - (int8_t)dst_file;
        if (diff == 2 || diff == -2) {
            p_move_list.Add(Move::Castle(p_src_sq, dst_sq));
        } else {
            p_move_list.Add(Move::Normal(p_src_sq, dst_sq));
        }
    }
}

MoveList MoveGen::PseudoMove(const Position& p_pos) {
    MoveList moves;

    const Color color = p_pos.SideToMove();
    const Square king_sq = p_pos.GetKing(color);

    // generate king moves first
    KingMoves<MoveMaskType::Move>(color, king_sq, p_pos, moves);

    // if two pieces check the king,
    // only moving king can resolve the check
    // no need to check other pieces
    const CheckerList& checkers = p_pos.State().checkers[color];
    const uint8_t checker_count = checkers.Count();
    if (checker_count == 2) {
        return moves;
    }

    const bool in_check = checker_count == 1;
    Square checker_sq{};
    PieceType checker_type{ PieceType::Null };
    if (in_check) {
        const auto checker = checkers.Get(0).unwrap();
        checker_sq = checker.square;
        checker_type = checker.type;
    }

    for (uint8_t i = 0; i < kPieceTypeMax; ++i) {
        const Piece piece = BuildPiece(static_cast<PieceType>(i), color);
        for (Square sq : p_pos.Bitboard(piece).Squares()) {
            PseudoFromSquare(p_pos, sq, piece, moves, king_sq, in_check, checker_sq, checker_type);
        }
    }
    return moves;
}

bool MoveGen::IsMoveLegal(Position& p_pos, Move p_move) {
    UndoState undo;
    const bool ok = p_pos.MakeMove(p_move, undo);
    p_pos.UnmakeMove(p_move, undo);
    return ok;
}

MoveList MoveGen::LegalMove(const Position& p_pos) {
    MoveList pseudo = PseudoMove(p_pos);
    MoveList moves;
    Position copy = p_pos;
    for (Move mv : pseudo) {
        if (IsMoveLegal(copy, mv)) {
            moves.Add(mv);
        }
    }
    return moves;
}

void MoveGen::PseudoFromSquare(const Position& p_pos,
                               Square p_from,
                               Piece p_piece,
                               MoveList& p_move_list,
                               Square p_king_sq,
                               bool p_in_check,
                               Square p_checker_sq,
                               PieceType p_checker_type) {
    constexpr MoveMaskType MV_TYPE = MoveMaskType::Move;

    const Color color = p_pos.SideToMove();
    const Bitboard friendly = p_pos.m_state.occupancies[color];
    const Bitboard enemy = p_pos.m_state.occupancies[FlipColor(color)];
    const PieceType piece_type = GetType(p_piece);

    if (piece_type == PieceType::Pawn) {
        PawnMoves<MV_TYPE>(color,
                           p_pos,
                           p_from,
                           p_move_list,
                           p_king_sq,
                           p_in_check,
                           p_checker_sq,
                           p_checker_type);
        return;
    }

    Bitboard mask(0);
    switch (piece_type) {
        case PieceType::Knight: {
            mask = KnightMask<MV_TYPE>(p_from, friendly);
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
        if (!p_in_check || ResolveCheck(sq, p_checker_sq, p_king_sq)) {
            p_move_list.Add(Move::Normal(p_from, sq));
        }
    }
}

void MoveGen::AttackMapAndCheckers(const Position& p_pos,
                                   Color p_color,
                                   Bitboard& p_out_attack,
                                   CheckerList& p_out_checkers) {
    p_out_checkers.Clear();

    const Color enemy_color = FlipColor(p_color);
    const Bitboard enemy_king_mask = p_pos.Bitboard(BuildPiece(PieceType::King, enemy_color));

    const Piece pawn = BuildPiece(PieceType::Pawn, p_color);
    const Piece knight = BuildPiece(PieceType::Knight, p_color);
    const Piece bishop = BuildPiece(PieceType::Bishop, p_color);
    const Piece rook = BuildPiece(PieceType::Rook, p_color);
    const Piece queen = BuildPiece(PieceType::Queen, p_color);
    const Piece king = BuildPiece(PieceType::King, p_color);

    const Bitboard friendly = p_pos.State().occupancies[p_color];
    const Bitboard enemy = p_pos.State().occupancies[enemy_color];

    cave::EnumArray<PieceType, Bitboard, 6> masks;

    for (Square sq : p_pos.Bitboard(pawn).Squares()) {
        constexpr PieceType type = PieceType::Pawn;
        Bitboard attack_mask = PawnMask<MoveMaskType::Attack>(p_color, p_pos, sq);
        masks[type] |= attack_mask;
        if ((attack_mask & enemy_king_mask).Any()) {
            p_out_checkers.Add(sq, type);
        }
    }

    for (Square sq : p_pos.Bitboard(knight).Squares()) {
        constexpr PieceType type = PieceType::Knight;
        Bitboard attack_mask = KnightMask<MoveMaskType::Attack>(sq, friendly);
        masks[type] |= attack_mask;
        if ((attack_mask & enemy_king_mask).Any()) {
            p_out_checkers.Add(sq, type);
        }
    }

    for (Square sq : p_pos.Bitboard(bishop).Squares()) {
        constexpr PieceType type = PieceType::Bishop;
        Bitboard attack_mask = BishopMask<MoveMaskType::Attack>(sq, friendly, enemy);
        masks[type] |= attack_mask;
        if ((attack_mask & enemy_king_mask).Any()) {
            p_out_checkers.Add(sq, type);
        }
    }

    for (Square sq : p_pos.Bitboard(rook).Squares()) {
        constexpr PieceType type = PieceType::Rook;
        Bitboard attack_mask = RookMask<MoveMaskType::Attack>(sq, friendly, enemy);
        masks[type] |= attack_mask;
        if ((attack_mask & enemy_king_mask).Any()) {
            p_out_checkers.Add(sq, type);
        }
    }

    for (Square sq : p_pos.Bitboard(queen).Squares()) {
        constexpr PieceType type = PieceType::Queen;
        Bitboard attack_mask = QueenMask<MoveMaskType::Attack>(sq, friendly, enemy);
        masks[type] |= attack_mask;
        if ((attack_mask & enemy_king_mask).Any()) {
            p_out_checkers.Add(sq, type);
        }
    }

    for (Square sq : p_pos.Bitboard(king).Squares()) {
        constexpr PieceType type = PieceType::King;
        Bitboard attack_mask = KingMask<MoveMaskType::Attack>(
            p_color,
            sq,
            p_pos);
        masks[type] |= attack_mask;
        // impossible to capture king with king,
        // no need to add king as a checker
    }

    p_out_attack = masks[PieceType::Pawn] |
                   masks[PieceType::Knight] |
                   masks[PieceType::Bishop] |
                   masks[PieceType::Rook] |
                   masks[PieceType::Queen] |
                   masks[PieceType::King];
}

static_assert(kPawnAttackMasks[0][8] == Bitboard(1llu << 17));

}  // namespace chess::core
