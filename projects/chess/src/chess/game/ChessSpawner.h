#pragma once
#include "cave/core/memory/Pointer.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace chess {

class ChessViewFactory;

enum class SpawnType : uint8_t {
    Menu,
    Gameplay,
};

class Spawner {
    using Entity = cave::ecs::Entity;

public:
    Spawner(cave::SceneQuery& query, cave::SceneCommandWriter& writer);
    ~Spawner();

    // @TODO: pass FEN instead
    void spawnPieces(SpawnType type);

private:
    void spawnExtraPieces(Entity piece_parent);
    void spawnTiles();

    cave::SceneQuery& m_query;
    cave::SceneCommandWriter& m_writer;

    cave::Owner<ChessViewFactory> m_factory;

    Entity m_offset_node;

    bool m_prev_no_save = false;
};

}  // namespace chess