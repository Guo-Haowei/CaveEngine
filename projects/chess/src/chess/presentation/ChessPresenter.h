#pragma once
#include <array>
#include "cave/runtime/ecs/Entity.h"

#include "chess/core/Position.h"
#include "chess/presentation/ChessBoardView.h"
#include "chess/presentation/ChessPieceView.h"

// clang-format off
namespace cave { class IHostServices; }
// clang-format on

namespace chess {

class ChessPresenter {
public:
    ChessPresenter(cave::IHostServices& host) noexcept;

    void present();

    void initialize();

    void redrawBoard(const core::Position& position);

    void applyMove(const core::Position& position, core::Move mv);

    // @TODO: refactor
    ChessBoardView board_view_;
private:
    cave::IHostServices& host_;

    ChessPieceView piece_view_;
};

}  // namespace chess
