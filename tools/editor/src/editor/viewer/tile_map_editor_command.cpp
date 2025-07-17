#include "tile_map_editor_command.h"

namespace cave {

std::shared_ptr<SetTileCommand> SetTileCommand::AddTile(TileMapAsset& p_tile_map, TileIndex p_index, TileId p_tile) {
    Option<TileId> old_tile = p_tile_map.GetTile(p_index);

    if (!p_tile_map.AddTile(p_index, p_tile)) {
        return nullptr;
    }

    p_tile_map.IncRevision();

    auto cmd = std::make_shared<SetTileCommand>();
    cmd->m_old_tile = old_tile;
    cmd->m_new_tile = p_tile;
    cmd->m_index = p_index;
    return cmd;
}

std::shared_ptr<SetTileCommand> SetTileCommand::RemoveTile(TileMapAsset& p_tile_map, TileIndex p_index) {
    Option<TileId> old_tile = p_tile_map.GetTile(p_index);

    if (!p_tile_map.RemoveTile(p_index)) {
        return nullptr;
    }

    p_tile_map.IncRevision();

    auto cmd = std::make_shared<SetTileCommand>();
    cmd->m_old_tile = old_tile;
    cmd->m_new_tile = Option<TileId>::None();
    cmd->m_index = p_index;
    return cmd;
}

bool SetTileCommand::MergeCommand(const UndoCommand* p_other) {
    if (auto other = dynamic_cast<const SetTileCommand*>(p_other); other) {
        return other->m_index == m_index &&
               other->m_new_tile == m_new_tile &&
               other->m_old_tile == m_old_tile &&
               other->m_handle.GetGuid() == m_handle.GetGuid();
    }
    return false;
}

bool SetTileCommand::SetTile(Option<TileId> p_new_tile) {
    TileMapAsset* tile_map = m_handle.Get();
    if (!tile_map) {
        return false;
    }
    const bool ok = p_new_tile.is_none() ? tile_map->RemoveTile(m_index) : tile_map->AddTile(m_index, p_new_tile.unwrap());
    DEV_ASSERT(ok);
    tile_map->IncRevision();
    return ok;
}

}  // namespace cave
