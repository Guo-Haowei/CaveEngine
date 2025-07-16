#pragma once
#include "engine/tile_map/tile_map_asset.h"
#include "editor/undo_redo/undo_command.h"

namespace cave {

// @TODO: abstract brush class
class SetTileCommand : public UndoCommand {
public:
    bool Undo() override {
        return SetTile(m_old_tile);
    }

    bool Redo() override {
        return SetTile(m_new_tile);
    }

    static std::shared_ptr<SetTileCommand> AddTile(TileMapAsset& p_tile_map, TileIndex p_index, TileId p_tile);

    static std::shared_ptr<SetTileCommand> RemoveTile(TileMapAsset& p_tile_map, TileIndex p_index);

    bool MergeCommand(const UndoCommand* p_other) override;

    void SetHandle(Handle<TileMapAsset>&& p_handle) {
        m_handle = std::move(p_handle);
    }

private:
    bool SetTile(Option<TileId> p_new_tile);

    Handle<TileMapAsset> m_handle;
    TileIndex m_index;

    Option<TileId> m_old_tile;
    Option<TileId> m_new_tile;
};

}  // namespace cave
