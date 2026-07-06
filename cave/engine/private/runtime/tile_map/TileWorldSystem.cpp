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

namespace {

struct TileRange {
    int16_t min_x = 0;
    int16_t min_y = 0;
    int16_t max_x = 0;
    int16_t max_y = 0;
};

inline TileRange GetTileRangeFromAABB(const Box2& aabb, float tile_size) {
    TileCoord min_tile = TileWorldSystem::worldToTile(aabb.min(), tile_size);
    TileCoord max_tile = TileWorldSystem::worldToTile(aabb.max(), tile_size);

    return TileRange{
        min_tile.x,
        min_tile.y,
        max_tile.x,
        max_tile.y
    };
}

}  // namespace

TileWorldSystem::TileWorldSystem()
    : m_debug_id(MakeDebugId(this)) {}

void TileWorldSystem::start(SceneContext& ctx) {
    rebuildCollision(ctx);
}

TileCoord TileWorldSystem::worldToTile(Vec2f world_pos, float tile_size) {
    return TileCoord{
        static_cast<int16_t>(std::floor(world_pos.x / tile_size)),
        static_cast<int16_t>(std::floor(world_pos.y / tile_size))
    };
}

std::vector<TileHit> TileWorldSystem::querySolidTiles(const math::Box2& aabb) const {
    std::vector<TileHit> result;

    const float tile_size = 1.0f;
    TileRange range = GetTileRangeFromAABB(aabb, tile_size);

    for (int16_t y = range.min_y; y <= range.max_y; ++y) {
        for (int16_t x = range.min_x; x <= range.max_x; ++x) {
            TileCoord coord{ x, y };
            if (!isSolid(coord)) {
                continue;
            }

            Vec2f tile_min{
                static_cast<float>(x) * tile_size,
                static_cast<float>(y) * tile_size
            };

            Vec2f tile_max = tile_min + Vec2f(tile_size, tile_size);

            Box2 tile_aabb{
                tile_min,
                tile_max,
            };

            result.push_back({ coord, tile_aabb });
        }
    }

    return result;
}

void TileWorldSystem::rebuildCollision(SceneContext& ctx) {
    m_world_bound.invalidate();

    auto view = ctx.scene.view<TileMapInstanceComponent, TransformComponent>();
    for (auto [ent, instance, transform] : view) {
        TileMapAsset* tile_map = instance.tileMapHandle().get();
        Vec2f offset = transform.translation().xy;

        if (!tile_map) {
            CRASH_NOW_MSG("TileMapAsset is null");
            continue;
        }

        TileSetAsset* tile_set = tile_map->tileSetHandle().get();
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
                    m_rigid_tiles.addTile(coord, tile_id);

                    m_world_bound.expandToInclude(Vec2f{ coord.x, coord.y });
                }
            }
        }
    }

    if (m_world_bound.isValid()) {
        m_world_bound.setMinMax(m_world_bound.min(), m_world_bound.max() + Vec2f::One);
    }
}

}  // namespace cave
