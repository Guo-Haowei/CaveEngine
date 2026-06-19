// =============================================================================
// File: cave/runtime/tile_map/TileWorldSystem.h
// =============================================================================
#pragma once
#include "TileCoord.h"

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

    DebugId debugId() const override { return debug_id_; }

protected:
    void onAttach() override;
    void onDetach() override;

private:
    const DebugId debug_id_;
};

}  // namespace cave

