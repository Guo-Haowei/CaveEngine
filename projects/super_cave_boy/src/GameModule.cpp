#include "GameModule.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/core/math/AABB.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/display/IDebugDrawService.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"

#include "CameraController.h"
#include "PlayerController.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;

namespace {

// @TODO: refactor this part
Box2 GetPlayerAABB(const SceneQuery& query) {
    ecs::Entity ent = query.findFirstByName("player");

    auto transform = query.component<TransformComponent>(ent);
    auto collider = query.component<ColliderComponent>(ent);
    const Mat4f& m = transform->GetWorldMatrix();
    const Shape& shape = collider->shape();
    Vec4f min{ Vec2f::Zero - Vec2f(shape.data.half.xy), 0.0f, 1.0f };
    Vec4f max{ Vec2f::Zero + Vec2f(shape.data.half.xy), 0.0f, 1.0f };
    min = m * min;
    max = m * max;

    return Box2(min.xy, max.xy);
}

inline Box2 ExpandAABB(const Box2& aabb, Vec2f amount) {
    return Box2{
        aabb.Min() - amount,
        aabb.Max() + amount
    };
}

struct TileRange {
    int16_t min_x = 0;
    int16_t min_y = 0;
    int16_t max_x = 0;
    int16_t max_y = 0;
};

inline TileCoord WorldToTile(Vec2f world_pos, float tile_size) {
    return TileCoord{
        static_cast<int16_t>(std::floor(world_pos.x / tile_size)),
        static_cast<int16_t>(std::floor(world_pos.y / tile_size))
    };
}

inline TileRange GetTileRangeFromAABB(const Box2& aabb, float tile_size) {
    TileCoord min_tile = WorldToTile(aabb.Min(), tile_size);
    TileCoord max_tile = WorldToTile(aabb.Max(), tile_size);

    return TileRange{
        min_tile.x,
        min_tile.y,
        max_tile.x,
        max_tile.y
    };
}

}  // namespace

GameModule::GameModule() = default;
GameModule::~GameModule() = default;

void GameModule::registerNativeScripts(NativeScriptRegistry& registry) {
    registry.registerScript<PlayerController>("PlayerController");
    registry.registerScript<CameraController>("CameraController");
}

void GameModule::onModuleLoaded(IHostServices& host) {
    LOG_OK(LogChannel::Game, "GameModule Loaded");

    unused(host);
}

void GameModule::onModuleUnloaded(IHostServices&) {
}

void GameModule::onGameBegin(IHostServices& host) {
    unused(host);
}

void GameModule::onGameEnd(IHostServices& host) {
    unused(host);
}

void GameModule::tick(IHostServices& host, const FrameTime& time) {
    unused(time);

    SceneQuery& query = host.sceneQuery();

    IDebugDrawService& debug_draw = host.debugDraw();

    const TileWorldSystem* tile_world = query.system<TileWorldSystem>();
    DEV_ASSERT(tile_world);

    constexpr Vec4f kPlayerColor(0, 0, 1, 0.9f);
    constexpr Vec4f kTileColor(1, 0, 0, 0.9f);
    constexpr float tile_size = 1.0f;

    Box2 aabb = GetPlayerAABB(query);
    debug_draw.addBox2Frame(aabb.Min(), aabb.Max(), kPlayerColor, 0.04f);

    aabb = ExpandAABB(aabb, Vec2f(tile_size));
    TileRange range = GetTileRangeFromAABB(aabb, tile_size);

    for (int16_t y = range.min_y; y <= range.max_y; ++y) {
        for (int16_t x = range.min_x; x <= range.max_x; ++x) {
            if (!tile_world->isSolid(TileCoord(x, y))) {
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

            debug_draw.addBox2Frame(tile_aabb.Min(), tile_aabb.Max(), kTileColor);
        }
    }
}

}  // namespace super_cave_boy
