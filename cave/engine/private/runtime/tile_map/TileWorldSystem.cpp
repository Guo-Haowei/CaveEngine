#include "cave/core/diagnostics/DebugIdAllocator.h"

#include <deque>

#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/tile_map/TileMapAsset.h"
#include "cave/runtime/tile_map/TileMapInstanceComponent.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"
#include "cave/runtime/tile_map/TileSetAsset.h"
#include "cave/runtime/scene/SceneRuntime.h"

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

TileWorldSystem::TileWorldSystem(SceneRuntime& runtime)
    : ISceneSystem(runtime)
    , m_debug_id(MakeDebugId(this)) {}

TileWorldSystem::~TileWorldSystem() = default;

void TileWorldSystem::start() {
    rebuildCollision();
}

TileCoord TileWorldSystem::worldToTile(Vec2f world_pos, float tile_size) {
    return TileCoord{
        static_cast<int16_t>(std::floor(world_pos.x / tile_size)),
        static_cast<int16_t>(std::floor(world_pos.y / tile_size))
    };
}

Vec2f TileWorldSystem::tileToWorld(TileCoord coord, float tile_size) {
    return {
        (coord.x + 0.5f) * tile_size,
        (coord.y + 0.5f) * tile_size,
    };
}

Vector<TileHit> TileWorldSystem::querySolidTiles(const math::Box2& aabb) const {
    Vector<TileHit> result;

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

TilePath TileWorldSystem::findPath(TileCoord start, TileCoord goal) const {
    if (start == goal) {
        return {};
    }

    HashMap<TileCoord, Option<TileCoord>> visited;
    visited[start] = Some(start);

    std::deque<TileCoord> ready{ start };

    constexpr std::array<TileCoord, 4> directions = {
        TileCoord{ -1, 0 },
        TileCoord{ +1, 0 },
        TileCoord{ 0, -1 },
        TileCoord{ 0, +1 },
    };

    while (!ready.empty()) {
        TileCoord coord = ready.front();
        if (coord == goal) break;

        ready.pop_front();

        for (TileCoord dir : directions) {
            const TileCoord next = coord + dir;

            if (m_rigid_tiles.tileAt(next).is_some()) continue;

            auto [it, ok] = visited.try_emplace(next, Some(coord));
            if (!ok) continue;

            ready.push_back(next);
        }
    }

    TilePath path;
    for (TileCoord cursor = goal; cursor != start;) {
        path.push_back(cursor);

        auto it = visited.find(cursor);
        if (it == visited.end()) {
            return {};
        }

        if (!DEV_VERIFY(it->second.is_some())) {
            return {};
        }

        cursor = it->second.unwrap_unchecked();
    }

    std::reverse(path.begin(), path.end());
    return path;
}

void TileWorldSystem::rebuildCollision() {
    m_world_bound.invalidate();

    auto rebuild_layer = [this](const TileMapLayer& layer, Vec2f offset) {
        TileSetAsset* tile_set = layer.handle().get();
        if (!tile_set) {
            CRASH_NOW_MSG("TileSetAsset is null");
            return;
        }

        for (auto&& [chunk_coord, chunk] : layer.chunks().chunks()) {
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
    };

    auto view = m_runtime.scene().view<TileMapInstanceComponent, TransformComponent>();
    for (auto [ent, instance, transform] : view) {
        TileMapAsset* tile_map = instance.tileMapHandle().get();
        Vec2f offset = transform.translation().xy;

        if (!DEV_VERIFY(tile_map)) {
            continue;
        }

        for (const TileMapLayer& layer : tile_map->layers()) {
            rebuild_layer(layer, offset);
        }
    }

    if (m_world_bound.valid()) {
        m_world_bound.setMinMax(m_world_bound.min(), m_world_bound.max() + Vec2f::One);
    }
}

}  // namespace cave
