#include "cave/core/diagnostics/DebugIdAllocator.h"

#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/tile_map/TileMapAsset.h"
#include "cave/runtime/tile_map/TileMapInstanceComponent.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"


// @TODO: make TileSetAsset public
#include "engine/private/runtime/assets/TileSetAsset.h"

// @TODO: refactor
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

TileWorldSystem::TileWorldSystem()
    : debug_id_(MakeDebugId(this)) {}

void TileWorldSystem::onAttach() {
    rebuildCollision();
}

void TileWorldSystem::onDetach() {
    collision_tiles_.chunks().clear();
}

void TileWorldSystem::rebuildCollision() {
    auto view = context().scene.view<TileMapInstanceComponent, TransformComponent>();
    for (auto [ent, instance, transform] : view) {
        TileMapAsset* tile_map = instance.tileMapHandle().Get();
        if (!tile_map) {
            CRASH_NOW_MSG("TileMapAsset is null");
            continue;
        }

        TileSetAsset* tile_set = tile_map->tileSetHandle().Get();
        if (!tile_map) {
            CRASH_NOW_MSG("TileSetAsset is null");
            continue;
        }

        for (auto&& [coord, chunk] : tile_map->tiles().chunks()) {
            if (!chunk) {
                continue;
            }

            for (int16_t y = 0; y < kTileChunkSize; ++y) {
                for (int16_t x = 0; x < kTileChunkSize; ++x) {
                    TileId tile_id = chunk->at(x, y);
                    auto res = tile_set->getCollider(tile_id);
                    if (res.is_none()) continue;
                    Shape shape = res.unwrap_unchecked();
                    DEV_ASSERT(shape.type == ShapeType::Box);

                    DEV_ASSERT(0 && "add tile");
                }
            }
        }
    }
}

}  // namespace cave
