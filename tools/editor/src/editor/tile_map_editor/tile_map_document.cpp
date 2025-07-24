#include "tile_map_document.h"

#include "tile_map_editor.h"
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

    Option<TileId> m_old_tile{ None() };
    Option<TileId> m_new_tile{ None() };
};

std::shared_ptr<SetTileCommand> SetTileCommand::AddTile(TileMapAsset& p_tile_map, TileIndex p_index, TileId p_tile) {
    Option<TileId> old_tile = p_tile_map.GetTile(p_index);

    if (!p_tile_map.AddTile(p_index, p_tile)) {
        return nullptr;
    }

    p_tile_map.IncRevision();

    auto cmd = std::make_shared<SetTileCommand>();
    cmd->m_old_tile = old_tile;
    cmd->m_new_tile = Some(p_tile);
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
    cmd->m_new_tile = None();
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
    const bool ok = p_new_tile.is_none() ? tile_map->RemoveTile(m_index) : tile_map->AddTile(m_index, p_new_tile.unwrap_unchecked());
    DEV_ASSERT(ok);
    tile_map->IncRevision();
    return ok;
}

void TileMapDocument::RequestAdd(const Vector2f& p_cursor, const TileId& p_id) {
    TileIndex tile;
    if (m_tile_map_editor.CursorToTile(p_cursor, tile)) {
        m_commands.emplace_back(CommandAddTile{ tile, p_id });
    }
}

void TileMapDocument::RequestErase(const Vector2f& p_cursor) {
    TileIndex tile;
    if (m_tile_map_editor.CursorToTile(p_cursor, tile)) {
        m_commands.emplace_back(CommandEraseTile{ tile });
    }
}

void TileMapDocument::FlushCommands() {
    auto handle = GetHandle<TileMapAsset>();
    TileMapAsset* tile_map = handle.Get();
    DEV_ASSERT(tile_map);
    if (!tile_map) return;

    // process commands
    for (const auto& command : m_commands) {
        std::visit([&](auto&& p_cmd) {
            using T = std::decay_t<decltype(p_cmd)>;
            if constexpr (std::is_same_v<T, CommandAddTile>) {
                if (auto cmd = SetTileCommand::AddTile(*tile_map, p_cmd.tile, p_cmd.id); cmd) {
                    m_dirty = true;
                    cmd->SetHandle(std::move(handle));
                    m_undo_stack->Submit(cmd);
                }
            } else if constexpr (std::is_same_v<T, CommandEraseTile>) {
                if (auto cmd = SetTileCommand::RemoveTile(*tile_map, p_cmd.tile); cmd) {
                    m_dirty = true;
                    cmd->SetHandle(std::move(handle));
                    m_undo_stack->Submit(cmd);
                }
            }
        },
                   command);
    }

    m_commands.clear();
}

}  // namespace cave
