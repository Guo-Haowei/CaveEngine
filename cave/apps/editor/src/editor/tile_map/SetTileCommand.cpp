#include "SetTileCommand.h"

#include "cave/runtime/tile_map/TileMapLayerComponent.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

bool SetTileCommand::apply(IDocument&) {
    Scene* scene = resolveScene(m_scene_id);
    TileMapLayerComponent* layer = scene->component<TileMapLayerComponent>(m_enity);
    if (!layer) {
        return false;
    }

    ChunkedTileData& tiles = layer->chunks();
    for (const auto& cmd : m_cmds) {
        [[maybe_unused]]
        bool ok = cmd.after.is_some()
                      ? tiles.addTile(cmd.coord, cmd.after.unwrap_unchecked())
                      : tiles.removeTile(cmd.coord);
        DEV_ASSERT(ok);
    }

    layer->resolveAllTerrain();
    layer->updateTileCache();
    return true;
}

bool SetTileCommand::undo(IDocument&) {
    Scene* scene = resolveScene(m_scene_id);
    TileMapLayerComponent* layer = scene->component<TileMapLayerComponent>(m_enity);
    if (!layer) {
        return false;
    }

    ChunkedTileData& tiles = layer->chunks();
    for (auto cmd = m_cmds.rbegin(); cmd != m_cmds.rend(); ++cmd) {
        [[maybe_unused]]
        bool ok = cmd->before.is_some()
                      ? tiles.addTile(cmd->coord, cmd->before.unwrap_unchecked())
                      : tiles.removeTile(cmd->coord);
        DEV_ASSERT(ok);
    }

    layer->resolveAllTerrain();
    layer->updateTileCache();
    return true;
}

}  // namespace cave
