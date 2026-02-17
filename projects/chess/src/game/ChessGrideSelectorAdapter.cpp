#include "ChessGrideSelectorAdapter.h"

#include "ChessGame.h"
#include "ChessPresenter.h"

namespace chess {

using core::Color;
using core::Move;
using core::Square;

bool ChessGridSelectorAdapter::CanSelect(int x, int y) {
    const Square sq = Square::FromFileRank((uint8_t)x, (uint8_t)y);

    return m_game.SideToMove() == m_game.ColorAt(sq);
}

void ChessGridSelectorAdapter::OnSelect(int x, int y) {
    const Square sq = Square::FromFileRank((uint8_t)x, (uint8_t)y);

    std::span<const Move> moves = m_game.LegalMovesFromSquare(sq);

    core::Bitboard bb;
    for (Move mv : moves) {
        bb.Set(mv.to);
    }
    m_presenter.HighlightSquares(bb);
}

bool ChessGridSelectorAdapter::CanDrop(int sx, int sy, int dx, int dy) {
    (void)sx;
    (void)sy;
    (void)dx;
    (void)dy;
    return false;
}

void ChessGridSelectorAdapter::OnDrop(int sx, int sy, int dx, int dy) {
    (void)sx;
    (void)sy;
    (void)dx;
    (void)dy;
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
