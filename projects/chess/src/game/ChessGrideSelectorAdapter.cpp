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
    (void)sq;

    std::span<const Move> moves = m_game.LegalMovesFromSquare(sq);

    for (Move mv : moves) {
        m_presenter.HighlightSquare(mv.to);
    }
}

bool ChessGridSelectorAdapter::CanDrop(int sx, int sy, int dx, int dy) {
    (void)sx;
    (void)sy;
    (void)dx;
    (void)dy;
    return true;
}

void ChessGridSelectorAdapter::OnDrop(int sx, int sy, int dx, int dy) {
    (void)sx;
    (void)sy;
    (void)dx;
    (void)dy;
}

void ChessGridSelectorAdapter::OnCancel() {
}

void ChessGridSelectorAdapter::OnInvalid(int sx, int sy, int dx, int dy) {
    (void)sx;
    (void)sy;
    (void)dx;
    (void)dy;
}

}  // namespace chess
