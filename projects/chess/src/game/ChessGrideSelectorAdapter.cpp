#include "ChessGrideSelectorAdapter.h"

#include "ChessGameClient.h"
#include "ChessMatchAuthority.h"
#include "ChessPresenter.h"

namespace chess {

using core::Color;
using core::Move;
using core::Square;

bool ChessGridSelectorAdapter::CanSelect(int x, int y) {
    const Square sq = Square::FromFileRank((uint8_t)x, (uint8_t)y);

    const core::Position& pos = m_client.Pos();
    return pos.SideToMove() == pos.ColorAt(sq);
}

void ChessGridSelectorAdapter::OnSelect(int x, int y) {
    const Square sq = Square::FromFileRank((uint8_t)x, (uint8_t)y);

    std::span<const Move> moves = m_client.LegalMovesFromSquare(sq);

    core::Bitboard bb;
    for (Move mv : moves) {
        bb.Set(mv.to);
    }
    m_presenter.HighlightSquares(bb);
}

bool ChessGridSelectorAdapter::CanDrop(int sx, int sy, int dx, int dy) {
    const Square sq = Square::FromFileRank((uint8_t)sx, (uint8_t)sy);

    std::span<const Move> moves = m_client.LegalMovesFromSquare(sq);
    for (Move mv : moves) {
        const auto [from_file, from_rank] = mv.from.FileRank();
        const auto [to_file, to_rank] = mv.to.FileRank();

        if (from_file == sx && from_rank == sy && to_file == dx && to_rank && dy) {
            return true;
        }
    }

    return false;
}

void ChessGridSelectorAdapter::OnDrop(int sx, int sy, int dx, int dy) {
    m_presenter.HighlightSquares({});

    const core::Position& pos = m_client.Pos();

    Move move{
        Square::FromFileRank((uint8_t)sx, (uint8_t)sy),
        Square::FromFileRank((uint8_t)dx, (uint8_t)dy),
    };

    const PlayerId id = pos.SideToMove() == core::Color::White ? 0 : 1;

    core::UndoState undo;
    pos.MakeMove(move, undo);
}

void ChessGridSelectorAdapter::OnCancel() {
    m_presenter.HighlightSquares({});
}

void ChessGridSelectorAdapter::OnInvalid(int sx, int sy, int dx, int dy) {
    (void)sx;
    (void)sy;
    (void)dx;
    (void)dy;
}

}  // namespace chess
