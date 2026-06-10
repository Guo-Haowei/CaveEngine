#pragma once
#include <array>
#include "cave/runtime/ecs/Entity.h"

#include "chess/core/Position.h"

// clang-format off
namespace cave { class IHostServices; }
namespace cave { class SceneCommandWriter; }
// clang-format on

namespace chess {

class ChessBoardView {
    using Entity = ::cave::ecs::Entity;

public:
    ChessBoardView(cave::IHostServices& host) noexcept;

    void initialize();

    void drawBoard();

    void setHighlight(core::Bitboard bitboard) {
        highlights_ = bitboard;
    }

    void setHovered(core::Square square) {
        hovered_square_ = square;
    }

private:
    cave::IHostServices& host_;
    cave::SceneCommandWriter& writer_;

    core::Square hovered_square_{ 0 };

    Entity selector_;

    core::Bitboard highlights_{};
    std::array<Entity, 64> tiles_;
};

}  // namespace chess
