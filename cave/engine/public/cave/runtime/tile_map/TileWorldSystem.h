// =============================================================================
// File: cave/runtime/tile_map/TileWorldSystem.h
// =============================================================================
#pragma once
#include "TileData.h"

#include "cave/core/math/Box.h"
#include "cave/runtime/scene/ISceneSystem.h"

namespace cave {

struct TileHit {
    TileCoord coord{};
    math::Box2 aabb;
};

class TileWorldSystem final : public ISceneSystem {
    CAVE_SCENE_SYSTEM(SceneSystemId::TileWorld)

public:
    TileWorldSystem(SceneRuntime& runtime);
    ~TileWorldSystem() override;

    const ChunkedTileData& rigidTiles() const { return m_rigid_tiles; }

    bool isSolid(TileCoord coord) const {
        return m_rigid_tiles.tileAt(coord).is_some();
    }

    std::vector<TileHit> querySolidTiles(const math::Box2& aabb) const;

    const math::Box2 worldBound() const { return m_world_bound; }

    static TileCoord worldToTile(math::Vec2f world_pos, float tile_size = 1.0f);

private:
    void update(SceneTickContext&) override {}

    void start() override;

    DebugId debugId() const override { return m_debug_id; }

    SceneTickDomain domain() const override { return SceneTickDomain::Simulate; }

    void rebuildCollision();

    const DebugId m_debug_id;

    ChunkedTileData m_rigid_tiles;
    math::Box2 m_world_bound;
};

}  // namespace cave
