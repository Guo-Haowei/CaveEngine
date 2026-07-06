#pragma once
#include <array>
#include <vector>

#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/scene/SceneQuery.h"

#include "chess/core/Position.h"

namespace chess {

class ChessPieceView {
    using Entity = ::cave::ecs::Entity;

    struct Entry {
        std::vector<Entity> pool{};
        uint8_t cursor{ 0 };

        Entity getAndAdvance();
    };

public:
    ChessPieceView(cave::SceneQuery& query)
        : m_query(query) {}

    void initialize();

    void redrawPieces(const core::Position& position);

    void spawnPiece(core::Piece piece, core::Square square);

    void removePiece(core::Square square);

    void movePiece(core::Square from, core::Square to);

    void applyMove(const core::Position& position, core::Move mv);

private:
    Entity entityAt(core::Square square) const { return m_board[square.index()]; }

    cave::SceneQuery& m_query;

    std::array<Entity, 64> m_board;
    std::array<Entry, core::kPieceMax> m_piece_pool;
};

}  // namespace chess
