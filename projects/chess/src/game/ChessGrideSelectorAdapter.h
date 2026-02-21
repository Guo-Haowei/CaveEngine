#pragma once
#include <span>
#include "core/Move.h"

namespace chess {

class ChessGameClient;
class ChessPresenter;

class ChessGridSelectorAdapter {
public:
    explicit ChessGridSelectorAdapter(
        ChessGameClient& p_game,
        ChessPresenter& p_presenter) noexcept
        : m_client(p_game)
        , m_presenter(p_presenter) {
    }

    bool CanSelect(int x, int y);
    void OnSelect(int x, int y);
    bool CanDrop(int sx, int sy, int dx, int dy);
    void OnDrop(int sx, int sy, int dx, int dy);
    void OnCancel();
    void OnInvalid(int sx, int sy, int dx, int dy);

private:
    ChessGameClient& m_client;
    ChessPresenter& m_presenter;
};

}  // namespace chess
