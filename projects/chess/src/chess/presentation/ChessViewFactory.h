#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/math/Vec.h"

#include "chess/core/Piece.h"
#include "chess/core/Square.h"

// clang-format off
namespace cave { class SceneCommandWriter; }
// clang-format on

namespace chess {

struct PieceCreatInfo {
    core::Piece piece;
    cave::ecs::Entity parent;
    bool visible = true;
};

struct TileCreatInfo {
    cave::math::Vec4f color;
    const char* name;
    cave::ecs::Entity parent;
    bool visible = false;
};

class ChessViewFactory {
    using Entity = ::cave::ecs::Entity;

public:
    ChessViewFactory(cave::SceneCommandWriter& writer);

    Entity createPiece(core::Square square, const PieceCreatInfo& info);
    Entity createTile(core::Square square, const TileCreatInfo& info);

private:
    cave::SceneCommandWriter& m_writer;
    const char* m_materials[2];

    std::array<uint8_t, core::kPieceMax> m_piece_counters{ 0 };
};

}  // namespace chess
