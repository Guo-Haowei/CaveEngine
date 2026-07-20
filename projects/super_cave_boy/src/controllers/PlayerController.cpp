#include "PlayerController.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/core/error/ErrorMacros.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/render/components/SpriteRendererComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/game/GameSession.h"
#include "cave/runtime/game/platformer/FacingComponent.h"
#include "cave/runtime/input/IGameInput.h"
#include "cave/runtime/scene/MotorSystem.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/tile_map/TileMapInstanceComponent.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::literals;
using namespace ::cave::math;
using ::cave::ecs::Entity;

namespace {

constexpr float kPlayerBounceSpeed = 10.f;

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
    FacingComponent& facing,
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
            facing.facing = Facing::Right;
            return true;
        }

        if (NearlyEqual(body.min().x, solid.max().x, kPlayerGrabEps)) {
            facing.facing = Facing::Left;
            return true;
        }
    }

    return false;
}

}  // namespace

void PlayerController::start() {
    m_animator = query().findChildByName("animator_node", entity());

    m_health = session().getInt(kPlayerHealthID);

    message().listen(kPlayerDamagedID, [this](const Message& message) {
        if (m_state_machine.is(PlayerState::Normal)) {
            PlayerHurtInfo info{ .damage = 1,
                                 .entity = message.sender };
            takeDamage(info);
        }
    });

    message().listen(kPlayerBouncedID, [this](const Message&) {
        if (m_state_machine.is(PlayerState::Normal)) {
            bounceFromEnemy(kPlayerBounceSpeed);
        }
    });

    message().listen(kPlayerLeaveID, [this](const Message&) {
        m_block_input = true;
        m_state_machine.switchTo(PlayerState::Exiting);
    });

    message().listen(kCutsceneStartID, [this](const Message&) {
        m_block_input = true;
    });

    message().listen(kCutsceneEndID, [this](const Message&) {
        m_block_input = false;
    });

    m_state_machine.addState(
        PlayerState::Normal,
        {
            .update = std::bind_front(&PlayerController::updateNormal, this),
        });

    m_state_machine.addState(
        PlayerState::Exiting,
        {
            .update = std::bind_front(&PlayerController::updateExiting, this),
            .on_enter = std::bind_front(&PlayerController::onEnterExiting, this),
        });

    m_state_machine.switchTo(PlayerState::Normal);
}

void PlayerController::update(float dt) {
    m_state_machine.update(dt);
}

void PlayerController::updateNormal(float dt) {
    m_hurt_timer.tick(dt);

    if (m_health <= 0) {
        // @TODO: do something
    }

    const IGameInput& input = services().gameInput();

    auto transform = component<TransformComponent>();
    auto collider = component<ColliderComponent>();
    auto vel = component<VelocityComponent>();
    auto motor = component<MotorComponent>();
    auto contact = component<ContactComponent>();
    auto animator = query().component<SpriteAnimatorComponent>(m_animator);

    DEV_ASSERT(transform && collider && vel && motor && contact && animator);

    if (hurt()) {
        updatePlayerState(*vel);
        updateAnimation(*animator);
        return;
    }

    bool jump_pressed = false;
    int move_x = 0;
    if (!m_block_input) {
        jump_pressed = input.isPressed("ui_up"_sid);
        move_x = input.isPressed("ui_right"_sid) - input.isPressed("ui_left"_sid);
    }

    // Contact is from previous MotorSystem frame.
    m_taking_jump = contact->hit_down;

    if (contact->hit_down) {
        m_grabbing = false;
    }

    // Jump can cancel grab.
    if (jump_pressed) {
        tryJump(*vel, *motor);
    }

    // If currently grabbing, freeze player and skip normal movement.
    if (m_grabbing) {
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
    const bool airborne = !m_taking_jump;
    const bool falling = vel->linear.y < 0.0f;

    if (airborne && falling) {
        const float predicted_dy = vel->linear.y * dt;
        const Box2 body = ComputeWorldAABB(*transform, *collider);

        const TileWorldSystem* tile_world = system<TileWorldSystem>();
        auto* facing = component<FacingComponent>();
        DEV_ASSERT(tile_world && facing);
        if (CheckWallGrab(body, predicted_dy, *facing, *tile_world)) {
            m_grabbing = true;
            m_taking_jump = false;

            vel->linear.x = 0.0f;
            vel->linear.y = 0.0f;

            motor->affected_by_gravity = false;
        }
    }

    updatePlayerState(*vel);
    updateAnimation(*animator);
}

void PlayerController::onEnterExiting() {
    auto* velocity = component<VelocityComponent>();
    if (DEV_VERIFY(velocity)) {
        velocity->linear.x = 0.0f;
    }
    auto* anim = query().component<SpriteAnimatorComponent>(m_animator);
    if (DEV_VERIFY(anim)) {
        anim->pause();
    }
}

void PlayerController::updateExiting(float) {
    auto* sprite = query().component<SpriteRendererComponent>(m_animator);
    if (DEV_VERIFY(sprite)) {
        const float ratio = 1.0f - (m_state_machine.stateTime() / kExitAnimationDuration);
        sprite->tintColor().a = math::clamp(ratio, 0.0f, 1.0f);
    }
}

void PlayerController::updateAnimation(SpriteAnimatorComponent& animator) {
    switch (m_state) {
        case PlayerNormalState::Idle: {
            animator.currentClip("idle");
        } break;
        case PlayerNormalState::Walk: {
            animator.currentClip("walk");
        } break;
        case PlayerNormalState::Air: {
            animator.currentClip("jump");
            // if (motor_.vspeed > 0.0f) {
            // } else {
            //     animator->currentClip("jump");
            // }
        } break;
        case PlayerNormalState::Grab: {
            animator.currentClip("grab");
        } break;
        case PlayerNormalState::Hurt: {
            animator.currentClip("hurt");
        } break;
    }
}

void PlayerController::updatePlayerState(VelocityComponent& vel) {
    if (hurt()) {
        m_state = PlayerNormalState::Hurt;
        return;
    }

    if (m_grabbing) {
        m_state = PlayerNormalState::Grab;
        return;
    }

    if (!m_taking_jump) {
        m_state = PlayerNormalState::Air;
        return;
    }

    if (vel.linear.x != 0.0f) {
        m_state = PlayerNormalState::Walk;
        return;
    }

    m_state = PlayerNormalState::Idle;
}

void PlayerController::tryJump(VelocityComponent& vel,
                               MotorComponent& motor) {
    if (m_grabbing) {
        vel.linear.y = kPlayerWallJumpForce;

        m_taking_jump = false;
        m_grabbing = false;

        motor.affected_by_gravity = true;
        return;
    }

    if (m_taking_jump) {
        vel.linear.y = kPlayerJumpForce;

        m_taking_jump = false;
        m_grabbing = false;

        motor.affected_by_gravity = true;
        return;
    }
}

void PlayerController::takeDamage(const PlayerHurtInfo& info) {
    if (m_hurt_timer.active()) {
        return;
    }

    m_health -= 1;
    session().setInt(kPlayerHealthID, m_health);

    m_hurt_timer.start();

    const auto* enemy_transform = query().component<TransformComponent>(info.entity);
    const auto* transform = component<TransformComponent>();
    auto* vel = component<VelocityComponent>();
    auto* motor = component<MotorComponent>();
    if (!DEV_VERIFY(enemy_transform && transform && vel && motor)) {
        return;
    }

    const float source_x = enemy_transform->translation().x;
    const float player_x = transform->translation().x;
    const float direction = player_x >= source_x ? 1.0f : -1.0f;

    vel->linear.x = direction * kPlayerKnockbackX;
    vel->linear.y = kPlayerKnockbackY;

    motor->affected_by_gravity = true;

    m_grabbing = false;
    m_taking_jump = false;

    // @TODO: play sound
}

void PlayerController::bounceFromEnemy(float bounce_speed) {
    auto* vel = component<VelocityComponent>();
    auto* motor = component<MotorComponent>();

    vel->linear.y = bounce_speed;
    motor->affected_by_gravity = true;

    m_grabbing = false;
    m_taking_jump = false;
}

void PlayerController::onBodyEntered(Entity ent) {
    onBodyStay(ent);
}

void PlayerController::onBodyStay(Entity ent) {
    const auto* collider = query().component<ColliderComponent>(ent);
    const auto* transform = query().component<TransformComponent>(ent);
    if (collider && transform && IsLava(*collider)) {
        Vec2f lava_center = transform->translation().xy;
        PlayerHurtInfo info{ 1, ent };
        takeDamage(info);
    }
}

}  // namespace super_cave_boy
