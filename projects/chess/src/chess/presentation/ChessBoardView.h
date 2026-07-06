#pragma once
#include <array>

#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/scene/SceneQuery.h"

#include "chess/core/Position.h"

namespace chess {

class ChessBoardView {
    using Entity = ::cave::ecs::Entity;

public:
    ChessBoardView(cave::SceneQuery& query)
        : m_query(query) {}

    void initialize();

    void drawBoard();

    void setHighlight(core::Bitboard bitboard) {
        m_highlights = bitboard;
    }

    void setHovered(core::Square square) {
        m_hovered_square = square;
    }

private:
    cave::SceneQuery& m_query;

    core::Square m_hovered_square{ 0 };

    Entity m_selector;

    core::Bitboard m_highlights{};
    std::array<Entity, 64> m_tiles;
};

}  // namespace chess
