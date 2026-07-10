#include "SetTileCommand.h"

#include "editor/document/IDocument.h"

#include "cave/runtime/tile_map/TileMapAsset.h"

namespace cave {

bool SetTileCommand::apply(IDocument& doc) {
    TileMapAsset* tile_map = doc.handle<TileMapAsset>().get();
    if (!tile_map) {
        return false;
    }

    ChunkedTileData& tiles = tile_map->tiles();

    for (const auto& cmd : m_cmds) {
        [[maybe_unused]]
        bool ok = cmd.after.is_some()
                      ? tiles.addTile(cmd.coord, cmd.after.unwrap_unchecked())
                      : tiles.removeTile(cmd.coord);
        DEV_ASSERT(ok);
    }

    tile_map->incRevision();
    return true;
}

bool SetTileCommand::undo(IDocument& doc) {
    TileMapAsset* tile_map = doc.handle<TileMapAsset>().get();
    if (!tile_map) {
        return false;
    }

    ChunkedTileData& tiles = tile_map->tiles();

    for (auto cmd = m_cmds.rbegin(); cmd != m_cmds.rend(); ++cmd) {
        [[maybe_unused]]
        bool ok = cmd->before.is_some()
                      ? tiles.addTile(cmd->coord, cmd->before.unwrap_unchecked())
                      : tiles.removeTile(cmd->coord);
        DEV_ASSERT(ok);
    }

    tile_map->incRevision();
    return true;
}

}  // namespace cave
