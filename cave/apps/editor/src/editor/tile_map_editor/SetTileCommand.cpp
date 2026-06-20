#include "SetTileCommand.h"

#include "editor/document/IDocument.h"

namespace cave {

bool SetTileCommand::setTile(IDocument& doc, Option<TileId> tile) {
    auto tile_map_handle = doc.handle<TileMapAsset>();

    TileMapAsset* tile_map = tile_map_handle.Get();
    if (!tile_map) {
        return false;
    }

    bool ok;
    if (tile.is_some()) {
        ok = tile_map->tiles().addTile(index_, tile.unwrap_unchecked());
    } else {
        ok = tile_map->tiles().removeTile(index_);
    }

    tile_map->incRevision();
    return ok;
}

bool SetTileCommand::canCoalesceWith(const IEditCmd* cmd) const {
    if (auto other = dynamic_cast<const SetTileCommand*>(cmd); other) {
        bool ok = index_ == other->index_;
        ok = ok && ent_ == other->ent_;
        ok = ok && old_tile_ == other->old_tile_;
        ok = ok && new_tile_ == other->new_tile_;
        return ok;
    }
    return false;
}

void SetTileCommand::coalesceFrom(std::unique_ptr<IEditCmd> cmd) {
    unused(cmd);
}

}  // namespace cave
