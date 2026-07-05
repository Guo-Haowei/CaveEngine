#include "SnakeController.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/MovementComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/platformer/FacingComponent.h"
#include "cave/runtime/scene/MotorSystem.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;

namespace {

constexpr float kSnakeSpeed = 2.0f;
constexpr float kProbeEps = 0.05f;

bool ShouldTurnAround(const Box2& body,
                      int facing_x,
                      const TileWorldSystem& world) {
    DEV_ASSERT(facing_x == -1 || facing_x == 1);

    const float front_x = facing_x > 0
                              ? body.max().x + kProbeEps
                              : body.min().x - kProbeEps;

    const float wall_y = (body.min().y + body.max().y) * 0.5f;

    const float ground_y = body.min().y - kProbeEps;

    const TileCoord wall_tile = TileWorldSystem::worldToTile({ front_x, wall_y });
    const TileCoord ground_tile = TileWorldSystem::worldToTile({ front_x, ground_y });

    const bool wall_in_front = world.isSolid(wall_tile);
    const bool ground_missing = !world.isSolid(ground_tile);

    return wall_in_front || ground_missing;
}

}  // namespace

void SnakeController::onCreate(SceneContext& ctx) {
    auto facing = ctx.query.component<FacingComponent>(entity());
    // @TODO: prefab override
    switch (facing->facing) {
        case Facing::Left: {
            m_facing_x = -1;
        } break;
        case Facing::Right: {
            m_facing_x = 1;
        } break;
        default: {
            LOG_ERROR("Invalid facing value {}", EnumTraits<Facing>::ToString(facing->facing));
            m_facing_x = -1;
        } break;
    }
}

void SnakeController::onUpdate(cave::SceneContext& ctx, float dt) {
    SceneQuery& query = ctx.query;

    auto transform = query.component<TransformComponent>(entity());
    auto collider = query.component<ColliderComponent>(entity());
    auto vel = query.component<VelocityComponent>(entity());

    DEV_ASSERT(transform && collider && vel);

    const TileWorldSystem* tile_world = query.system<TileWorldSystem>();
    DEV_ASSERT(tile_world);

    const Box2 body = ComputeWorldAABB(*transform, *collider);

    if (ShouldTurnAround(body, m_facing_x, *tile_world)) {
        m_facing_x = -m_facing_x;
    }

    vel->linear.x = static_cast<float>(m_facing_x) * kSnakeSpeed;

    const float dx = vel->linear.x * dt;

    transform->translate({ dx, 0.0f, 0.0f });
}

}  // namespace super_cave_boy
