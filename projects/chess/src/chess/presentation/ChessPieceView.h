#pragma once
#include <array>
#include <vector>

#include "cave/runtime/ecs/Entity.h"

#include "chess/core/Position.h"

// clang-format off
namespace cave { class IHostServices; }
namespace cave { class SceneCommandWriter; }
// clang-format on

namespace chess {

class ChessPieceView {
    using Entity = ::cave::ecs::Entity;

public:
    ChessPieceView(cave::IHostServices& host) noexcept;

    void redrawBoard(const core::Position& position);

    void spawnPiece(core::Piece piece, core::Square square);
    void removePiece(core::Square square);

    void movePiece(core::Square from, core::Square to);

    Entity entityAt(core::Square square) const { return board_[square.index()]; }

    void initialize();
private:
    struct Entry {
        std::vector<Entity> pool{};
        uint8_t cursor{ 0 };

        Entity getAndAdvance();
    };

    cave::IHostServices& host_;
    cave::SceneCommandWriter& writer_;

    std::array<Entity, 64> board_;
    std::array<Entry, core::kPieceMax> piece_pool_;
};

}  // namespace chess
