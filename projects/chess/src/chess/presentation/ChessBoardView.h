#pragma once
#include <array>

#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/scene/SceneQuery.h"

#include "chess/core/Position.h"

namespace chess {

class ChessBoardView {
    using Entity = ::cave::ecs::Entity;

public:
    void initialize(cave::SceneQuery& query);

    void drawBoard(cave::SceneQuery& query);

    void setHighlight(core::Bitboard bitboard) {
        highlights_ = bitboard;
    }

    void setHovered(core::Square square) {
        hovered_square_ = square;
    }

private:
    core::Square hovered_square_{ 0 };

    Entity selector_;

    core::Bitboard highlights_{};
    std::array<Entity, 64> tiles_;
};

}  // namespace chess
