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

void MoveGen::Pseudo(const Position& p_pos,
                     MoveList& p_out) {
    const Color stm = p_pos.SideToMove();

    for (uint8_t i = 0; i < kPieceTypeMax; ++i) {
        const Piece piece = BuildPiece(static_cast<PieceType>(i), stm);
        for (Square sq : p_pos.Bitboard(piece).Squares()) {
            PseudoFrom(p_pos, sq, piece, p_out);
        }
    }
}

void MoveGen::PseudoFrom(const Position& p_pos,
                         Square p_from,
                         Piece p_piece,
                         MoveList& p_out) {
    // @NOTE: assume all pieces moves like rooks
    (void)p_pos;
    (void)p_piece;

    const auto [file, rank] = p_from.FileRank();
    const Bitboard bb = MASK_FILES[file] | MASK_RANKS[rank];

    for (Square sq : bb.Squares()) {
        p_out.push_back({ p_from, sq });
    }
}

}  // namespace chess::core
