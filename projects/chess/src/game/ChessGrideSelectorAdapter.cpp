#include "ChessGrideSelectorAdapter.h"

#include "ChessGame.h"

namespace chess {

using core::Color;
using core::Square;

bool ChessGridSelectorAdapter::CanSelect(int x, int y) {
    const Square sq = Square::From((uint8_t)x, (uint8_t)y);

    const auto& pos = m_game.Position();
    const Color sq_color = pos.ColorAt(sq);
    if (pos.SideToMove() == sq_color) {
        return true;
    }

    return false;
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
