#include "PlayerController.h"

#include "cave/core/error/ErrorMacros.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/ecs/components/SpriteAnimatorComponent.h"
#include "cave/runtime/ecs/components/MovementComponent.h"
#include "cave/runtime/display/IDebugDrawService.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/input/IGameInput.h"
#include "cave/runtime/scene/SceneContext.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/tile_map/TileMapInstanceComponent.h"

#include "platformer/PlatformerCollision.h"

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

inline Box2 ExpandAABB(const Box2& aabb, Vec2f amount) {
    return Box2{
        aabb.Min() - amount,
        aabb.Max() + amount
    };
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
    query.SetMinMax(
        query.Min() + Vec2f{ -kGrabEps, dy },
        query.Max() + Vec2f{ kGrabEps, 0.0f });

    for (const TileHit& hit_tile : world.querySolidTiles(query)) {
        const Box2& solid = hit_tile.aabb;

        if (!IsLedgeTile(world, hit_tile)) {
            continue;
        }

        const bool player_top_crosses_tile_top =
            body.Max().y >= solid.Max().y &&
            body.Max().y + dy <= solid.Max().y;

        if (!player_top_crosses_tile_top) {
            continue;
        }

        if (NearlyEqual(body.Max().x, solid.Min().x, kGrabEps)) {
            return true;
        }

        if (NearlyEqual(body.Min().x, solid.Max().x, kGrabEps)) {
            return true;
        }
    }

    return false;
}

void Land(LegacyPlayerMotor& motor, VelocityComponent& vel) {
    if (vel.linear.y == 0.0f) {
        return;
    }
    vel.linear.y = 0.0f;

    motor.taking_jump = true;
    motor.grabbing = false;
}

void TryJump(VelocityComponent& vel,
             MotorComponent& motor,
             LegacyPlayerMotor& player) {
    if (player.grabbing) {
        vel.linear.y = kWallJumpForce;

        player.taking_jump = false;
        player.grabbing = false;

        motor.affected_by_gravity = true;
        return;
    }

    if (player.taking_jump) {
        vel.linear.y = kJumpForce;

        player.taking_jump = false;
        player.grabbing = false;

        motor.affected_by_gravity = true;
        return;
    }
}

}  // namespace

void PlayerController::onCreate() {
    const SceneQuery query(context().scene);

    animator_ = query.findChildByName("animator_node", entity());
}

void PlayerController::onUpdate(float dt) {
    const IGameInput& input = context().game_input;
    SceneQuery query(context().scene);

    auto transform = query.component<TransformComponent>(entity());
    auto collider = query.component<ColliderComponent>(entity());
    auto vel = query.component<VelocityComponent>(entity());
    auto motor = query.component<MotorComponent>(entity());
    auto contact = query.component<ContactComponent>(entity());

    DEV_ASSERT(transform && collider && vel && motor && contact);

    const TileWorldSystem* tile_world = query.system<TileWorldSystem>();
    DEV_ASSERT(tile_world);

    const bool jump_pressed = input.isPressed("ui_up"_sid);

    int move_x =
        input.isPressed("ui_right"_sid) -
        input.isPressed("ui_left"_sid);

    // Contact is from previous MotorSystem frame.
    motor_.taking_jump = contact->hit_down;

    if (contact->hit_down) {
        motor_.grabbing = false;
    }

    // Jump can cancel grab.
    if (jump_pressed) {
        TryJump(*vel, *motor, motor_);
    }

    // If currently grabbing, freeze player and skip normal movement.
    if (motor_.grabbing) {
        move_x = 0;

        vel->linear.x = 0.0f;
        vel->linear.y = 0.0f;

        motor->affected_by_gravity = false;

        updatePlayerState(*vel);
        updateAnimation(query);
        return;
    }

    // Normal horizontal control.
    vel->linear.x = move_x * motor_.speed;
    motor->affected_by_gravity = true;

    // Try starting wall grab.
    //
    // Must happen after normal velocity is known, but before MotorSystem.
    // MotorSystem will run after this script update.
    const bool airborne = !motor_.taking_jump;
    const bool falling = vel->linear.y < 0.0f;

    if (!motor_.hurt && airborne && falling) {
        const float predicted_dy = vel->linear.y * dt;
        const Box2 body = ComputeWorldAABB(*transform, *collider);

        if (CheckWallGrab(body, predicted_dy, *tile_world)) {
            motor_.grabbing = true;
            motor_.taking_jump = false;

            vel->linear.x = 0.0f;
            vel->linear.y = 0.0f;

            motor->affected_by_gravity = false;
        }
    }

    updatePlayerState(*vel);
    updateAnimation(query);
}

void PlayerController::updateAnimation(SceneQuery& query) {
    auto animator = query.component<SpriteAnimatorComponent>(animator_);
    auto transform = query.component<TransformComponent>(entity());

    DEV_ASSERT(animator);
    DEV_ASSERT(transform);

    switch (motor_.state) {
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
    auto& motor = motor_;
    if (motor.hurt) {
        motor.state = PlayerState::Hurt;
        return;
    }

    if (motor.grabbing) {
        motor.state = PlayerState::Grab;
        return;
    }

    if (!motor.taking_jump) {
        motor.state = PlayerState::Air;
        return;
    }

    if (vel.linear.x != 0.0f) {
        motor.state = PlayerState::Walk;
        return;
    }

    motor.state = PlayerState::Idle;
}

}  // namespace super_cave_boy
