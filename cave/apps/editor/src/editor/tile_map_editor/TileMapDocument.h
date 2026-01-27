#pragma once
#include <variant>

#include "engine/private/math/geomath.h"
#include "engine/private/assets/tile_map_asset.h"
#include "editor/document/document.h"

namespace cave {

class TileMapEditor;

struct CommandAddTile {
    TileIndex tile;
    TileId id;
};

struct CommandEraseTile {
    TileIndex tile;
};

class TileMapDocument : public OldDocument {
    using Command = std::variant<CommandAddTile, CommandEraseTile>;

public:
    TileMapDocument(const Guid& p_guid, const TileMapEditor& p_tile_map_editor)
        : OldDocument(p_guid)
        , m_tile_map_editor(p_tile_map_editor) {}

    void RequestAdd(const Vector2f& p_cursor, const TileId& p_id);
    void RequestErase(const Vector2f& p_cursor);

    void FlushCommands();

private:
    const TileMapEditor& m_tile_map_editor;
    std::vector<Command> m_commands;
};

}  // namespace cave
