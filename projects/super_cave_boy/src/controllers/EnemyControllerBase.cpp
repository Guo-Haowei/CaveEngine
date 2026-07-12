#include "EnemyControllerBase.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/runtime/ecs/components/MovementComponent.h"
#include "cave/runtime/ecs/components/SpriteAnimatorComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/scene/MotorSystem.h"
#include "cave/runtime/script/native/NativeScriptSystem.h"

#include "SuperCaveBoyDefines.h"

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

void EnemyControllerBase::start() {
    m_player = findPlayer();
    m_animator = query().findChildByName("animator_node", entity());
}

void EnemyControllerBase::takeDamageFromPlayer(int damage) {
    if (DEV_VERIFY(m_health > 0)) {
        m_health -= damage;
        if (!alive()) {
            query().queueDestroy(entity());
        }
    }
}

void EnemyControllerBase::onBodyEntered(Entity player) {
    return onBodyStay(player);
}

void EnemyControllerBase::onBodyStay(Entity player) {
#if USING(ENABLE_ASSERT)
    auto* player_collider = query().component<ColliderComponent>(player);
    DEV_ASSERT(player_collider);
    DEV_ASSERT(IsPlayer(*player_collider));
#endif
    auto script_system = system<NativeScriptSystem>();
    DEV_ASSERT(script_system);

    auto* player_script = query().component<NativeScriptComponent>(player);
    DEV_ASSERT(player_script);
    if (!player_script) {
        return;
    }

    Entity enemy = entity();

    if (IsStompingEnemy(query(), player, enemy)) {
        message().emit(kPlayerBounced, enemy);
        takeDamageFromPlayer(1);
        return;
    }

    auto* player_transform = query().component<TransformComponent>(player);
    auto* enemy_transform = component<TransformComponent>();
    DEV_ASSERT(player_transform && enemy_transform);

    const float player_x = player_transform->translation().x;
    const float enemy_x = enemy_transform->translation().x;

    const float dir_x = player_x >= enemy_x ? 1.0f : -1.0f;

    message().emit(kPlayerDamaged,
                   enemy,
                   Variant{
                       dir_x * kPlayerKnockbackX,
                       kPlayerKnockbackY,
                   });
}

Entity EnemyControllerBase::findPlayer() const {
    return query().findFirstByName("player");
}

void EnemyControllerBase::playAnimation(std::string_view name) {
    auto animator = query().component<SpriteAnimatorComponent>(m_animator);
    if (DEV_VERIFY(animator)) {
        animator->currentClip(name);
    }
}

}  // namespace super_cave_boy
