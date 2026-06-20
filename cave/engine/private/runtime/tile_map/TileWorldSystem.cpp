#include "cave/core/diagnostics/DebugIdAllocator.h"

#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/tile_map/TileMapAsset.h"
#include "cave/runtime/tile_map/TileMapInstanceComponent.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"
#include "cave/runtime/tile_map/TileSetAsset.h"

// @TODO: refactor
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using namespace ::cave::math;

TileWorldSystem::TileWorldSystem()
    : debug_id_(MakeDebugId(this)) {}

void TileWorldSystem::onAttach() {
    rebuildCollision();
}

void TileWorldSystem::onDetach() {
    rigid_tiles_.chunks().clear();
}

void TileWorldSystem::rebuildCollision() {
    auto view = context().scene.view<TileMapInstanceComponent, TransformComponent>();
    for (auto [ent, instance, transform] : view) {
        TileMapAsset* tile_map = instance.tileMapHandle().Get();
        Vec2f offset = transform.GetTranslation().xy;

        if (!tile_map) {
            CRASH_NOW_MSG("TileMapAsset is null");
            continue;
        }

        TileSetAsset* tile_set = tile_map->tileSetHandle().Get();
        if (!tile_map) {
            CRASH_NOW_MSG("TileSetAsset is null");
            continue;
        }

        for (auto&& [chunk_coord, chunk] : tile_map->tiles().chunks()) {
            if (!chunk) {
                continue;
            }

            for (int16_t y = 0; y < kTileChunkSize; ++y) {
                for (int16_t x = 0; x < kTileChunkSize; ++x) {
                    TileId tile_id = chunk->at(x, y);
                    if (tile_id == kEmptyTileId) continue;
                    auto res = tile_set->getCollider(tile_id);
                    if (res.is_none()) continue;
                    Shape shape = res.unwrap_unchecked();
                    DEV_ASSERT(shape.type == ShapeType::Box);

                    TileCoord coord;
                    coord.x = chunk_coord.x * kTileChunkSize + (int16_t)offset.x + x;
                    coord.y = chunk_coord.y * kTileChunkSize + (int16_t)offset.y + y;
                    rigid_tiles_.addTile(coord, tile_id);
                }
            }
        }
    }
}

}  // namespace cave
