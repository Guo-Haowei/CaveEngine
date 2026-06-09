#pragma once
#include <array>
#include "cave/runtime/ecs/Entity.h"

#include "chess/core/Position.h"
#include "chess/presentation/ChessPieceView.h"

// clang-format off
namespace cave { class IHostServices; }
namespace cave { class SceneCommandWriter; }
namespace cave { class SceneQuery; }
// clang-format on

namespace chess {

class ChessPresenter {
    using Entity = ::cave::ecs::Entity;

public:
    ChessPresenter(cave::IHostServices& host) noexcept;

    void present();

    void initialize();

    void redrawBoard(const core::Position& position);

    void applyMove(core::Color stm, core::Move mv);

    void setFocusedSquare(core::Square square) {
        focused_sq = square;
    }

    void setHighlightSquares(core::Bitboard bb) {
        highlights_ = bb;
    }

private:
    cave::IHostServices& host_;
    Entity selector_;

    core::Square focused_sq{ 0 };
    core::Bitboard highlights_;

    std::array<Entity, 64> tiles_;

    ChessPieceView piece_view_;
};

}  // namespace chess
