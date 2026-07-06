#include "EnemyControllerBase.h"
#include "PlayerController.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/scene/MotorSystem.h"
#include "cave/runtime/script/native/NativeScriptComponent.h"

#include "Utility.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;
using ::cave::ecs::Entity;

namespace {

bool IsStompingEnemy(SceneQuery& query, Entity player, Entity enemy) {
    auto* player_transform = query.component<TransformComponent>(player);
    auto* player_collider = query.component<ColliderComponent>(player);
    auto* player_velocity = query.component<VelocityComponent>(player);

    auto* enemy_transform = query.component<TransformComponent>(enemy);
    auto* enemy_collider = query.component<ColliderComponent>(enemy);

    if (!player_transform || !player_collider || !player_velocity ||
        !enemy_transform || !enemy_collider) {
        return false;
    }

    // Y-up convention:
    // falling downward means velocity.y < 0.
    if (player_velocity->linear.y >= 0.0f) {
        return false;
    }

    const Box2 player_box = ComputeWorldAABB(*player_transform, *player_collider);
    const Box2 enemy_box = ComputeWorldAABB(*enemy_transform, *enemy_collider);

    const float player_feet_y = player_box.min().y;
    const float enemy_top_y = enemy_box.max().y;

    // Since this runs after overlap is already detected, player's feet may
    // already be slightly inside enemy's box.
    return player_feet_y >= enemy_top_y - kPlayerStompTolerance;
}

}  // namespace

void EnemyControllerBase::start(SceneContext& ctx) {
    m_player = findPlayer(ctx.query);
    m_animator = ctx.query.findChildByName("animator_node", entity());
}

void EnemyControllerBase::destroy() {
}

void EnemyControllerBase::onBodyOverlapping(SceneContext& ctx, ecs::Entity player) {
    SceneQuery& query = ctx.query;
#if USING(ENABLE_ASSERT)
    auto* player_collider = query.component<ColliderComponent>(player);
    DEV_ASSERT(player_collider);
    DEV_ASSERT(IsPlayer(*player_collider));
#endif

    auto* player_script = query.component<NativeScriptComponent>(player);
    DEV_ASSERT(player_script && player_script->instance);
    PlayerController* controller = dynamic_cast<PlayerController*>(player_script->instance);
    DEV_ASSERT(controller);

    Entity enemy = entity();

    auto* player_velocity = query.component<VelocityComponent>(player);
    auto* player_motor = query.component<MotorComponent>(player);
    DEV_ASSERT(player_motor && player_velocity);
    if (IsStompingEnemy(query, player, enemy)) {
        controller->bounceFromEnemy(*player_velocity, *player_motor, kPlayerBounceSpeed);
        query.queueDestroy(enemy);
        return;
    }

    auto* player_transform = query.component<TransformComponent>(player);
    auto* enemy_transform = query.component<TransformComponent>(enemy);
    DEV_ASSERT(player_transform && enemy_transform);

    const float player_x = player_transform->translation().x;
    const float enemy_x = enemy_transform->translation().x;

    const float dir_x = player_x >= enemy_x ? 1.0f : -1.0f;

    PlayerHurtInfo hurt_info{
        .damage = 1,
        .knockback = math::Vec2f{
            dir_x * kPlayerKnockbackX,
            kPlayerKnockbackY,
        },
    };
    controller->takeDamage(*player_velocity, *player_motor, hurt_info);
}

Entity EnemyControllerBase::findPlayer(SceneQuery& query) const {
    return query.findFirstByName("player");
}

}  // namespace super_cave_boy
