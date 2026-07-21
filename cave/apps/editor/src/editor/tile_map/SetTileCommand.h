#pragma once
#include "cave/runtime/tile_map/TileData.h"

#include "editor/edit/EditCmdBase.h"

namespace cave {

class SetTileCommand : public EditCmdBase {
    struct Command {
        TileCoord coord;
        Option<TileId> before;
        Option<TileId> after;
    };

public:
    SetTileCommand(SceneRegistry& scene_reg,
                   SceneId scene_id,
                   ecs::Entity entity)
        : EditCmdBase(scene_reg)
        , m_scene_id(scene_id)
        , m_enity(entity) {
        DEV_ASSERT(entity.valid());
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
    SceneId m_scene_id;
    ecs::Entity m_enity;
};

}  // namespace cave
