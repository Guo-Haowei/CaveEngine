#pragma once
#include <span>
#include "core/Move.h"

namespace chess {

class ChessGame;
class ChessPresenter;

class ChessGridSelectorAdapter {
public:
    explicit ChessGridSelectorAdapter(
        ChessGame& p_game,
        ChessPresenter& p_presenter) noexcept
        : m_game(p_game)
        , m_presenter(p_presenter) {
    }

    bool CanSelect(int x, int y);
    void OnSelect(int x, int y);
    bool CanDrop(int sx, int sy, int dx, int dy);
    void OnDrop(int sx, int sy, int dx, int dy);
    void OnCancel();
    void OnInvalid(int sx, int sy, int dx, int dy);

private:
    ChessGame& m_game;
    ChessPresenter& m_presenter;
};

}  // namespace chess
