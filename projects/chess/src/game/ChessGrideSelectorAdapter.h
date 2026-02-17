#pragma once
#include <span>
#include "core/Move.h"

namespace chess {

class ChessGame;

class ChessGridSelectorAdapter {
public:
    explicit ChessGridSelectorAdapter(ChessGame& p_game) noexcept
        : m_game(p_game) {
    }

    bool CanSelect(int x, int y);
    void OnSelect(int x, int y);
    bool CanDrop(int sx, int sy, int dx, int dy);
    void OnDrop(int sx, int sy, int dx, int dy);
    void OnCancel();
    void OnInvalid(int sx, int sy, int dx, int dy);

private:
    ChessGame& m_game;

    std::span<const core::Move> m_cached_moves;
};

}  // namespace chess
