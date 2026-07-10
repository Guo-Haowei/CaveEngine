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
    SetTileCommand(SceneRegistry& scene_reg, ecs::Entity ent)
        : EditCmdBase(scene_reg, ent) {
    }

    void add(TileCoord coord, Option<TileId> before, Option<TileId> after) {
        m_cmds.emplace_back(coord, before, after);
    }

    bool empty() const { return m_cmds.empty(); }

    const char* label() const override { return "SetTileCommand"; }

    bool apply(IDocument& doc) override;

    bool undo(IDocument& doc) override;

    bool canCoalesceWith(const IEditCmd* cmd) const override;

    void coalesceFrom(Owner<IEditCmd> cmd) override;

private:
    bool setTiles(TileMapAsset& tile_map,
                  IDocument& doc,
                  size_t begin,
                  size_t end,
                  int inc);

    Vector<Command> m_cmds;
};

}  // namespace cave
