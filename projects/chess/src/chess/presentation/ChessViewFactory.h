#pragma once
#include "cave/core/math/Vector.h"
#include "cave/runtime/ecs/Entity.h"

#include "chess/core/Piece.h"

// clang-format off
namespace cave { class SceneCommandWriter; }
// clang-format on

namespace chess {

class ChessSpawner {
    using Entity = ::cave::ecs::Entity;
public:

    ChessSpawner(cave::SceneCommandWriter& writer, Entity parent);

    struct TileInitInfo {
        cave::math::Vector4f color;
        const char* name;
        bool visible;
        Entity parent;
    };

    void SpawnTile(uint8_t p_file,
                   uint8_t p_rank,
                   const TileInitInfo& p_info);

    void SpawnPiece(core::Piece p_piece, int p_file, int p_rank, int p_id);

    cave::SceneCommandWriter& writer_;
    Entity piece_parent;
    const char* materials[2];
};

}  // namespace chess
