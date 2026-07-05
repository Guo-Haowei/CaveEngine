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
    void initialize(cave::SceneQuery& query);

    void redrawPieces(cave::SceneQuery& query,
                      const core::Position& position);

    void spawnPiece(cave::SceneQuery& query,
                    core::Piece piece,
                    core::Square square);

    void removePiece(cave::SceneQuery& query,
                     core::Square square);

    void movePiece(cave::SceneQuery& query,
                   core::Square from,
                   core::Square to);

    void applyMove(cave::SceneQuery& query,
                   const core::Position& position,
                   core::Move mv);

private:
    Entity entityAt(core::Square square) const { return m_board[square.index()]; }

    std::array<Entity, 64> m_board;
    std::array<Entry, core::kPieceMax> m_piece_pool;
};

}  // namespace chess
