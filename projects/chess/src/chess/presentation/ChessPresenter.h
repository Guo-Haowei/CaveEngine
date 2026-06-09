#pragma once
#include <array>
#include "cave/runtime/ecs/Entity.h"

#include "chess/core/Position.h"

// @TODO: refactor?
#include "chess/presentation/ChessPieceRegistry.h"

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

    // ==== Board Representation ====
    void onBoot();

    void redrawBoard(const core::Position& position);

    void applyMove(core::Move mv);

    Entity getEntityAt(core::Square square) const {
        return board_[square.Index()];
    }

    // ==== Grid ====
    void setFocusedSquare(core::Square square) {
        focused_sq = square;
    }

    void setHighlightSquares(core::Bitboard bb) {
        highlights_ = bb;
    }

private:
    void clearSquare(cave::SceneCommandWriter& writer,
                     core::Square square);

    void movePiece(Entity ent, core::Square from, core::Square to);

    cave::IHostServices& host_;
    Entity selector_;

    core::Square focused_sq{ 0 };
    core::Bitboard highlights_;

    std::array<Entity, 64> tiles_;
    std::array<Entity, 64> board_;

    ChessPieceRegistry reg_;
};

}  // namespace chess
