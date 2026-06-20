// =============================================================================
// File: cave/runtime/tile_map/TileWorldSystem.h
// =============================================================================
#pragma once
#include "TileData.h"

#include "cave/runtime/scene/ISceneSystem.h"

namespace cave {

struct TileQueryResult {
    TileCoord coord{};
};

struct TileAabbQueryResult {
    bool hit = false;
    TileQueryResult first_hit{};
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

protected:
    void onAttach() override;
    void onDetach() override;

private:
    void rebuildCollision();

    const DebugId debug_id_;

    ChunkedTileData rigid_tiles_;
};

}  // namespace cave
