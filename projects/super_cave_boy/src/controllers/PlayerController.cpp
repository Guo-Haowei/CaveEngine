#include "PlayerController.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/core/error/ErrorMacros.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/SpriteAnimatorComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/display/IDebugDrawService.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/input/IGameInput.h"
#include "cave/runtime/scene/MotorSystem.h"
#include "cave/runtime/scene/SceneContext.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/tile_map/TileMapInstanceComponent.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::literals;
using namespace ::cave::math;
using ::cave::ecs::Entity;

namespace {

constexpr float kGravity = -35.0f;
constexpr float kJumpForce = 13.0f;
constexpr float kWallJumpForce = 9.5f;
constexpr float kGrabEps = 0.03f;

bool IsLedgeTile(const TileWorldSystem& world, const TileHit& hit) {
    TileCoord above = hit.coord;
    above.y += 1;

    return !world.isSolid(above);
}

inline bool NearlyEqual(float a, float b, float eps = 0.01f) {
    return std::abs(a - b) <= eps;
}

bool CheckWallGrab(
    const Box2& body,
    float dy,
    const TileWorldSystem& world) {
    if (dy >= 0.0f) {
        return false;
    }

    Box2 query = body;
    query.setMinMax(
        query.min() + Vec2f{ -kGrabEps, dy },
        query.max() + Vec2f{ kGrabEps, 0.0f });

    for (const TileHit& hit_tile : world.querySolidTiles(query)) {
        const Box2& solid = hit_tile.aabb;

        if (!IsLedgeTile(world, hit_tile)) {
            continue;
        }

        const bool player_top_crosses_tile_top =
            body.max().y >= solid.max().y &&
            body.max().y + dy <= solid.max().y;

        if (!player_top_crosses_tile_top) {
            continue;
        }

        if (NearlyEqual(body.max().x, solid.min().x, kGrabEps)) {
            return true;
        }

        if (NearlyEqual(body.min().x, solid.max().x, kGrabEps)) {
            return true;
        }
    }

    return false;
}

}  // namespace

void PlayerController::onCreate() {
    const SceneQuery query(context().scene);

    animator_ = query.findChildByName("animator_node", entity());
}

void PlayerController::onUpdate(float dt) {
    hurt_timer_.tick(dt);

    if (health_ <= 0) {
        // Later: revive/request respawn
        // Revive();
    }

    const IGameInput& input = context().game_input;
    SceneQuery query(context().scene);

    auto transform = query.component<TransformComponent>(entity());
    auto collider = query.component<ColliderComponent>(entity());
    auto vel = query.component<VelocityComponent>(entity());
    auto motor = query.component<MotorComponent>(entity());
    auto contact = query.component<ContactComponent>(entity());

    DEV_ASSERT(transform && collider && vel && motor && contact);

    if (hurt()) {
        updatePlayerState(*vel);
        updateAnimation(query);
        return;
    }

    const bool jump_pressed = input.isPressed("ui_up"_sid);

    int move_x =
        input.isPressed("ui_right"_sid) -
        input.isPressed("ui_left"_sid);

    // Contact is from previous MotorSystem frame.
    taking_jump_ = contact->hit_down;

    if (contact->hit_down) {
        grabbing_ = false;
    }

    // Jump can cancel grab.
    if (jump_pressed) {
        tryJump(*vel, *motor);
    }

    // If currently grabbing, freeze player and skip normal movement.
    if (grabbing_) {
        move_x = 0;

        vel->linear.x = 0.0f;
        vel->linear.y = 0.0f;

        motor->affected_by_gravity = false;

        updatePlayerState(*vel);
        updateAnimation(query);
        return;
    }

    // Normal horizontal control.
    vel->linear.x = move_x * kPlayerMoveX;
    motor->affected_by_gravity = true;

    // Try starting wall grab.
    //
    // Must happen after normal velocity is known, but before MotorSystem.
    // MotorSystem will run after this script update.
    const bool airborne = !taking_jump_;
    const bool falling = vel->linear.y < 0.0f;

    if (airborne && falling) {
        const float predicted_dy = vel->linear.y * dt;
        const Box2 body = ComputeWorldAABB(*transform, *collider);

        const TileWorldSystem* tile_world = query.system<TileWorldSystem>();
        DEV_ASSERT(tile_world);
        if (CheckWallGrab(body, predicted_dy, *tile_world)) {
            grabbing_ = true;
            taking_jump_ = false;

            vel->linear.x = 0.0f;
            vel->linear.y = 0.0f;

            motor->affected_by_gravity = false;
        }
    }

    updatePlayerState(*vel);
    updateAnimation(query);
}

void PlayerController::onCollision(ecs::Entity other) {
    SceneQuery query(context().scene);

    auto* other_collider = query.component<ColliderComponent>(other);
    if (!other_collider) {
        return;
    }

    if (!IsEnemy(*other_collider)) {
        return;
    }

#if 0
    if (isStompingEnemy(other)) {
        bounceFromEnemy();

        // enemy dies
        query.queueDestroy(other);
        return;
    }
#endif

    auto* player_transform = query.component<TransformComponent>(entity());
    auto* enemy_transform = query.component<TransformComponent>(other);
    if (!player_transform || !enemy_transform) {
        return;
    }

    const float player_x = player_transform->translation().x;
    const float enemy_x = enemy_transform->translation().x;

    const float dir_x = player_x >= enemy_x ? 1.0f : -1.0f;

    takeDamage(PlayerHurtInfo{
        .damage = 1,
        .knockback = math::Vec2f{
            dir_x * kKnockbackX,
            kKnockbackY,
        },
    });
}

void PlayerController::updateAnimation(SceneQuery& query) {
    auto animator = query.component<SpriteAnimatorComponent>(animator_);
    auto transform = query.component<TransformComponent>(entity());

    DEV_ASSERT(animator);
    DEV_ASSERT(transform);

    switch (state_) {
        case PlayerState::Idle:
            animator->currentClip("idle");
            break;

        case PlayerState::Walk:
            animator->currentClip("walk");
            break;

        case PlayerState::Air:
            animator->currentClip("jump");
            // if (motor_.vspeed > 0.0f) {
            // } else {
            //     animator->currentClip("jump");
            // }
            break;

        case PlayerState::Grab:
            animator->currentClip("grab");
            break;

        case PlayerState::Hurt:
            animator->currentClip("hurt");
            break;
    }
}

void PlayerController::updatePlayerState(VelocityComponent& vel) {
    if (hurt()) {
        state_ = PlayerState::Hurt;
        return;
    }

    if (grabbing_) {
        state_ = PlayerState::Grab;
        return;
    }

    if (!taking_jump_) {
        state_ = PlayerState::Air;
        return;
    }

    if (vel.linear.x != 0.0f) {
        state_ = PlayerState::Walk;
        return;
    }

    state_ = PlayerState::Idle;
}

void PlayerController::tryJump(VelocityComponent& vel,
                               MotorComponent& motor) {
    if (grabbing_) {
        vel.linear.y = kWallJumpForce;

        taking_jump_ = false;
        grabbing_ = false;

        motor.affected_by_gravity = true;
        return;
    }

    if (taking_jump_) {
        vel.linear.y = kJumpForce;

        taking_jump_ = false;
        grabbing_ = false;

        motor.affected_by_gravity = true;
        return;
    }
}

// @TODO: pass components
void PlayerController::takeDamage(const PlayerHurtInfo& info) {
    if (hurt_timer_.active()) {
        return;
    }

    health_ -= info.damage;

    hurt_timer_.start();

    SceneQuery query(context().scene);

    auto* velocity = query.component<VelocityComponent>(entity());
    auto* motor = query.component<MotorComponent>(entity());

    if (velocity) {
        velocity->linear.x = info.knockback.x;
        velocity->linear.y = info.knockback.y;
    }

    if (motor) {
        motor->affected_by_gravity = true;
    }

    grabbing_ = false;
    taking_jump_ = false;

    // @TODO: play sound
}

void PlayerController::bounceFromEnemy(float bounce_speed) {
    SceneQuery query(context().scene);
    auto* velocity = query.component<VelocityComponent>(entity());
    auto* motor = query.component<MotorComponent>(entity());

    if (velocity) {
        velocity->linear.y = bounce_speed;
    }

    if (motor) {
        motor->affected_by_gravity = true;
    }

    grabbing_ = false;
    taking_jump_ = false;
}

}  // namespace super_cave_boy
