#include "ChessGrideSelectorAdapter.h"

namespace chess {

bool ChessGridSelectorAdapter::CanSelect(int x, int y) {
    (void)x;
    (void)y;
    return true;
}

void ChessGridSelectorAdapter::OnSelect(int x, int y) {
    (void)x;
    (void)y;
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
