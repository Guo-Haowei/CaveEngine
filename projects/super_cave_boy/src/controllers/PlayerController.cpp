#include "PlayerController.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/core/error/ErrorMacros.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
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
    query = Box(query.min() + Vec2f{ -kPlayerGrabEps, dy },
                query.max() + Vec2f{ kPlayerGrabEps, 0.0f });

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

        if (NearlyEqual(body.max().x, solid.min().x, kPlayerGrabEps)) {
            return true;
        }

        if (NearlyEqual(body.min().x, solid.max().x, kPlayerGrabEps)) {
            return true;
        }
    }

    return false;
}

}  // namespace

void PlayerController::start(cave::SceneContext& ctx) {
    m_animator = ctx.query.findChildByName("animator_node", entity());
}

void PlayerController::update(cave::SceneContext& ctx, float dt) {
    m_hurt_timer.tick(dt);

    // if (health_ <= 0) {
    // }

    const IGameInput& input = ctx.services.gameInput();
    SceneQuery& query = ctx.query;

    auto transform = query.component<TransformComponent>(entity());
    auto collider = query.component<ColliderComponent>(entity());
    auto vel = query.component<VelocityComponent>(entity());
    auto motor = query.component<MotorComponent>(entity());
    auto contact = query.component<ContactComponent>(entity());
    auto animator = query.component<SpriteAnimatorComponent>(m_animator);

    DEV_ASSERT(transform && collider && vel && motor && contact && animator);

    if (hurt()) {
        updatePlayerState(*vel);
        updateAnimation(*animator);
        return;
    }

    const bool jump_pressed = input.isPressed("ui_up"_sid);

    int move_x =
        input.isPressed("ui_right"_sid) -
        input.isPressed("ui_left"_sid);

    // Contact is from previous MotorSystem frame.
    m_taking_jump_ = contact->hit_down;

    if (contact->hit_down) {
        m_grabbing_ = false;
    }

    // Jump can cancel grab.
    if (jump_pressed) {
        tryJump(*vel, *motor);
    }

    // If currently grabbing, freeze player and skip normal movement.
    if (m_grabbing_) {
        move_x = 0;

        vel->linear.x = 0.0f;
        vel->linear.y = 0.0f;

        motor->affected_by_gravity = false;

        updatePlayerState(*vel);
        updateAnimation(*animator);
        return;
    }

    // Normal horizontal control.
    vel->linear.x = move_x * kPlayerMoveX;
    motor->affected_by_gravity = true;

    // Try starting wall grab.
    //
    // Must happen after normal velocity is known, but before MotorSystem.
    // MotorSystem will run after this script update.
    const bool airborne = !m_taking_jump_;
    const bool falling = vel->linear.y < 0.0f;

    if (airborne && falling) {
        const float predicted_dy = vel->linear.y * dt;
        const Box2 body = ComputeWorldAABB(*transform, *collider);

        const TileWorldSystem* tile_world = query.system<TileWorldSystem>();
        DEV_ASSERT(tile_world);
        if (CheckWallGrab(body, predicted_dy, *tile_world)) {
            m_grabbing_ = true;
            m_taking_jump_ = false;

            vel->linear.x = 0.0f;
            vel->linear.y = 0.0f;

            motor->affected_by_gravity = false;
        }
    }

    updatePlayerState(*vel);
    updateAnimation(*animator);
}

void PlayerController::updateAnimation(SpriteAnimatorComponent& animator) {
    switch (m_state) {
        case PlayerState::Idle: {
            animator.currentClip("idle");
        } break;
        case PlayerState::Walk: {
            animator.currentClip("walk");
        } break;
        case PlayerState::Air: {
            animator.currentClip("jump");
            // if (motor_.vspeed > 0.0f) {
            // } else {
            //     animator->currentClip("jump");
            // }
        } break;
        case PlayerState::Grab: {
            animator.currentClip("grab");
        } break;
        case PlayerState::Hurt: {
            animator.currentClip("hurt");
        } break;
    }
}

void PlayerController::updatePlayerState(VelocityComponent& vel) {
    if (hurt()) {
        m_state = PlayerState::Hurt;
        return;
    }

    if (m_grabbing_) {
        m_state = PlayerState::Grab;
        return;
    }

    if (!m_taking_jump_) {
        m_state = PlayerState::Air;
        return;
    }

    if (vel.linear.x != 0.0f) {
        m_state = PlayerState::Walk;
        return;
    }

    m_state = PlayerState::Idle;
}

void PlayerController::tryJump(VelocityComponent& vel,
                               MotorComponent& motor) {
    if (m_grabbing_) {
        vel.linear.y = kPlayerWallJumpForce;

        m_taking_jump_ = false;
        m_grabbing_ = false;

        motor.affected_by_gravity = true;
        return;
    }

    if (m_taking_jump_) {
        vel.linear.y = kPlayerJumpForce;

        m_taking_jump_ = false;
        m_grabbing_ = false;

        motor.affected_by_gravity = true;
        return;
    }
}

void PlayerController::takeDamage(VelocityComponent& vel,
                                  MotorComponent& motor,
                                  const PlayerHurtInfo& info) {
    if (m_hurt_timer.active()) {
        return;
    }

    // health_ -= info.damage;

    m_hurt_timer.start();

    vel.linear.x = info.knockback.x;
    vel.linear.y = info.knockback.y;

    motor.affected_by_gravity = true;

    m_grabbing_ = false;
    m_taking_jump_ = false;

    // @TODO: play sound
}

void PlayerController::bounceFromEnemy(VelocityComponent& vel,
                                       MotorComponent& motor,
                                       float bounce_speed) {
    vel.linear.y = bounce_speed;
    motor.affected_by_gravity = true;

    m_grabbing_ = false;
    m_taking_jump_ = false;
}

}  // namespace super_cave_boy
