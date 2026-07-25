#include "SetTileCommand.h"

#include "cave/runtime/tile_map/TileMapLayerComponent.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

void SetTileCommand::record(TileCoord coord, Option<TileCell> before, Option<TileCell> after) {
    auto [it, inserted] = m_changes.try_emplace(coord, Change{ .before = before, .after = after });
    if (!inserted) it->second.after = after;
    if (it->second.before == it->second.after) m_changes.erase(it);
}

bool SetTileCommand::apply(IDocument&) {
    CRASH_NOW_MSG("should never call this");
#if 0
    Scene* scene = resolveScene(m_scene_id);
    TileMapLayerComponent* layer = scene->component<TileMapLayerComponent>(m_enity);
    if (!layer) {
        return false;
    }

    ChunkedTileData& tiles = layer->chunks();
    for (const auto& [coord, change] : m_changes) {
        [[maybe_unused]]
        bool ok = change.after.is_some()
                      ? tiles.addTile(coord, change.after.unwrap_unchecked())
                      : tiles.removeTile(coord);
        DEV_ASSERT(ok);
    }

    layer->resolveAllTerrain();
    layer->updateTileCache();
#endif
    return true;
}

bool SetTileCommand::undo(IDocument&) {
    Scene* scene = resolveScene(m_scene_id);
    TileMapLayerComponent* layer = scene->component<TileMapLayerComponent>(m_enity);
    if (!layer) {
        return false;
    }

    ChunkedTileData& tiles = layer->chunks();
    for (const auto& [coord, change] : m_changes) {
        [[maybe_unused]]
        bool ok = change.before.is_some()
                      ? tiles.addTile(coord, change.before.unwrap_unchecked())
                      : tiles.removeTile(coord);
        DEV_ASSERT(ok);
    }

    layer->resolveAllTerrain();
    layer->updateTileCache();
    return true;
}

}  // namespace cave
