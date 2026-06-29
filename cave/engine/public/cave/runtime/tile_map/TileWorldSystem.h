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
    TileWorldSystem();

    const ChunkedTileData& rigidTiles() const { return rigid_tiles_; }

    DebugId debugId() const override { return debug_id_; }

    bool isSolid(TileCoord coord) const {
        return rigid_tiles_.tileAt(coord).is_some();
    }

    std::vector<TileHit> querySolidTiles(const math::Box2& aabb) const;

    static TileCoord worldToTile(math::Vec2f world_pos, float tile_size = 1.0f);

protected:
    void onAttach(SceneContext& ctx) override;
    void onDetach(SceneContext& ctx) override;

private:
    void rebuildCollision(SceneContext& ctx);

    const DebugId debug_id_;

    ChunkedTileData rigid_tiles_;
};

}  // namespace cave
