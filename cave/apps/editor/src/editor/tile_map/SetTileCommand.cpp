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
    Scene* scene = resolveScene(m_scene_id);
    TileMapLayerComponent* layer = scene->component<TileMapLayerComponent>(m_enity);
    if (!layer) {
        return false;
    }

    for (const auto& [coord, change] : m_changes) {
        change.after.is_some()
            ? layer->setCell(coord, change.after.unwrap_unchecked())
            : layer->removeCell(coord);
    }
    return true;
}

bool SetTileCommand::undo(IDocument&) {
    Scene* scene = resolveScene(m_scene_id);
    TileMapLayerComponent* layer = scene->component<TileMapLayerComponent>(m_enity);
    if (!layer) {
        return false;
    }

    for (const auto& [coord, change] : m_changes) {
        change.before.is_some()
            ? layer->setCell(coord, change.before.unwrap_unchecked())
            : layer->removeCell(coord);
    }
    return true;
}

}  // namespace cave
