#include "ChessGrideSelectorAdapter.h"

#include "ChessGame.h"

namespace chess {

using core::Color;
using core::Move;
using core::Square;

bool ChessGridSelectorAdapter::CanSelect(int x, int y) {
    const Square sq = Square::From((uint8_t)x, (uint8_t)y);

    if (m_cached_moves.empty()) {
        m_cached_moves = m_game.LegalMovesFromSquare(sq);
    }

    return !m_cached_moves.empty();
}

void ChessGridSelectorAdapter::OnSelect(int x, int y) {
    const Square sq = Square::From((uint8_t)x, (uint8_t)y);
    (void)sq;
    // @TODO: legal move
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
