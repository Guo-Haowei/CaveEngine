#pragma once
#include "cave/runtime/tile_map/TileMapAsset.h"

#include "editor/edit/EditCmdBase.h"

namespace cave {

class SetTileCommand : public EditCmdBase {
public:
    SetTileCommand(SceneRegistry& scene_reg,
                   ecs::Entity ent,
                   TileCoord index,
                   Option<TileId> old_tile,
                   Option<TileId> new_tile)
        : EditCmdBase(scene_reg, ent)
        , index_(index)
        , old_tile_(old_tile)
        , new_tile_(new_tile) {
    }

    const char* label() const override { return "SetTileCommand"; }

    bool apply(IDocument& doc) override {
        return setTile(doc, new_tile_);
    }

    bool undo(IDocument& doc) override {
        return setTile(doc, old_tile_);
    }

    bool canCoalesceWith(const IEditCmd* cmd) const override;

    void coalesceFrom(std::unique_ptr<IEditCmd> cmd) override;

private:
    bool setTile(IDocument& doc, Option<TileId> tile);

    TileCoord index_;

    Option<TileId> old_tile_{ None() };
    Option<TileId> new_tile_{ None() };
};

}  // namespace cave
