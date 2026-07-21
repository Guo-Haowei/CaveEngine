#include "cave/core/diagnostics/DebugIdAllocator.h"

#include <deque>
#include <queue>

#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/tile_map/TileMapAsset.h"
#include "cave/runtime/tile_map/TileMapLayerComponent.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"
#include "cave/runtime/tile_map/TileSetAsset.h"
#include "cave/runtime/scene/SceneRuntime.h"

// @TODO: refactor
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using namespace ::cave::math;

namespace {

constexpr std::array<TileCoord, 4> kPathFindingDirections = {
    TileCoord{ -1, 0 },
    TileCoord{ +1, 0 },
    TileCoord{ 0, -1 },
    TileCoord{ 0, +1 },
};

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
    rebuildTiles();
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

TilePath TileWorldSystem::findPathAstar(TileCoord start, TileCoord goal) const {
    if (start == goal) {
        return {};
    }

    struct OpenNode {
        TileCoord coord;
        int priority = 0;  // f = g + h
        int cost = 0;      // g
    };

    struct CompareOpenNode {
        bool operator()(const OpenNode& lhs, const OpenNode& rhs) const {
            return lhs.priority > rhs.priority;
        }
    };

    auto heuristic = [](TileCoord a, TileCoord b) -> int {
        return std::abs(a.x - b.x) + std::abs(a.y - b.y);
    };

    struct Node {
        int cost = 0;
        Option<TileCoord> parent;
    };

    HashMap<TileCoord, Node> visited;
    visited[start] = { Node{ 0, Some(start) } };

    std::priority_queue<OpenNode, Vector<OpenNode>, CompareOpenNode> ready;
    ready.push(OpenNode{ start, heuristic(start, goal), 0 });

    while (!ready.empty()) {
        const OpenNode open_node = ready.top();
        ready.pop();

        auto current_it = visited.find(open_node.coord);
        DEV_ASSERT(current_it != visited.end());

        // Ignore an outdated queue entry.
        if (open_node.cost != current_it->second.cost) {
            continue;
        }

        if (open_node.coord == goal) {
            break;
        }

        for (TileCoord dir : kPathFindingDirections) {
            const TileCoord next = open_node.coord + dir;

            if (m_rigid_tiles.tileAt(next).is_some()) {
                continue;
            }

            const int new_cost = open_node.cost + 1;
            const int priority = new_cost + heuristic(next, goal);

            auto [it, inserted] = visited.try_emplace(
                next,
                Node{
                    .cost = new_cost,
                    .parent = Some(open_node.coord),
                });

            if (inserted) {
                ready.emplace(next, priority, new_cost);
                continue;
            }

            Node& node = it->second;

            if (new_cost < node.cost) {
                node.cost = new_cost;
                node.parent = Some(open_node.coord);

                ready.emplace(next, priority, new_cost);
            }
        }
    }

    TilePath path;
    for (TileCoord cursor = goal; cursor != start;) {
        path.push_back(cursor);

        auto it = visited.find(cursor);
        if (it == visited.end()) {
            return {};
        }

        if (!DEV_VERIFY(it->second.parent.is_some())) {
            return {};
        }

        cursor = it->second.parent.unwrap_unchecked();
    }

    std::reverse(path.begin(), path.end());
    return path;
}

void TileWorldSystem::handleTile(const TileDefinition& definition, TileCoord coord) {
    switch (definition.collision) {
        case CollisionType::Solid: {
            m_rigid_tiles.addTile(coord, static_cast<TileId>(definition.id));
            m_world_bound.expandToInclude(Vec2f{ coord.x, coord.y });
        } break;
        case CollisionType::Trigger: {
            Scene& scene = m_runtime.scene();
            const Vec2f local_min = definition.collision_shape.min();
            const Vec2f local_max = definition.collision_shape.max();
            const Vec2f local_center = (local_min + local_max) * 0.5f;
            const Vec2f local_size = local_max - local_min;

            const Vec2f world_center = { coord.x + local_center.x,
                                         coord.y + local_center.y };

            auto ent = scene.createEntity();
            auto& transform = scene.create<TransformComponent>(ent);
            transform.setTranslation(Vec3f(world_center, 0.0f));
            auto& collider = scene.create<ColliderComponent>(ent);
            collider.setLayer(definition.layer);
            collider.setMask(definition.mask);
            collider.setTrigger();
            collider.shape().data.half = Vec3f(0.5f * local_size, 0.5f);
        } break;
        default:
            break;
    }
}

void TileWorldSystem::rebuildTiles() {
    m_world_bound.invalidate();

    auto rebuild_layer = [this](const TileMapLayerComponent& layer,
                                int16_t offset_x,
                                int16_t offset_y) {
        TileSetAsset* tile_set = layer.tileSetHndle().get();
        if (!DEV_VERIFY(tile_set)) {
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
                    const TileDefinition* def = tile_set->getTileDefinition(tile_id);
                    if (!def) {
                        continue;
                    }
                    TileCoord coord{
                        chunk_coord.x * kTileChunkSize + offset_x + x,
                        chunk_coord.y * kTileChunkSize + offset_y + y,
                    };
                    handleTile(*def, coord);
                }
            }
        }
    };

    auto view = m_runtime.scene().view<TileMapLayerComponent, TransformComponent>();
    for (auto [ent, layer, transform] : view) {
        Vec2f offset = transform.translation().xy;

        rebuild_layer(layer, (int16_t)offset.x, (int16_t)offset.y);
    }

    if (m_world_bound.valid()) {
        m_world_bound.setMinMax(m_world_bound.min(), m_world_bound.max() + Vec2f::One);
    }
}

}  // namespace cave
