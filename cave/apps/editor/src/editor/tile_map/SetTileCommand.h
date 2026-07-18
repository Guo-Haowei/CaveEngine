#pragma once
#include "cave/runtime/tile_map/TileData.h"

#include "editor/edit/EditCmdBase.h"

namespace cave {

class TileMapAsset;

class SetTileCommand : public EditCmdBase {
    struct Command {
        TileCoord coord;
        Option<TileId> before;
        Option<TileId> after;
    };

public:
    SetTileCommand(SceneRegistry& scene_reg, int layer_id)
        : EditCmdBase(scene_reg, ecs::Entity::null())
        , layer_id(layer_id) {
        DEV_ASSERT(layer_id >= 0);
    }

    void add(TileCoord coord, Option<TileId> before, Option<TileId> after) {
        m_cmds.emplace_back(coord, before, after);
    }

    bool empty() const { return m_cmds.empty(); }

    const char* label() const override { return "SetTileCommand"; }

    bool apply(IDocument& doc) override;

    bool undo(IDocument& doc) override;

private:
    Vector<Command> m_cmds;
    int layer_id;
};

}  // namespace cave
