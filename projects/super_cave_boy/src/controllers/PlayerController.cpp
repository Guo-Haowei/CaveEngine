#include "PlayerController.h"

#include "cave/core/error/ErrorMacros.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/ecs/components/SpriteAnimatorComponent.h"
#include "cave/runtime/ecs/components/VelocityComponent.h"
#include "cave/runtime/display/IDebugDrawService.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/input/IGameInput.h"
#include "cave/runtime/scene/SceneContext.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/tile_map/TileMapInstanceComponent.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"

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
constexpr float kStepOffset = 0.05f;
constexpr float kGrabEps = 0.03f;
constexpr float kMinGroundSupport = 0.05f;  // tune; tile size is 1.0

bool IsLedgeTile(const TileWorldSystem& world, const TileHit& hit) {
    TileCoord above = hit.coord;
    above.y += 1;

    return !world.isSolid(above);
}

// @TODO: refactor this part
Box2 GetPlayerAABB(const SceneQuery& query) {
    ecs::Entity ent = query.findFirstByName("player");

    auto transform = query.component<TransformComponent>(ent);
    auto collider = query.component<ColliderComponent>(ent);
    return ComputeWorldAABB(*transform, *collider);
}

inline Box2 ExpandAABB(const Box2& aabb, Vec2f amount) {
    return Box2{
        aabb.Min() - amount,
        aabb.Max() + amount
    };
}

inline bool Overlap1DStrict(float a_min, float a_max, float b_min, float b_max) {
    return a_max > b_min && a_min < b_max;
}

inline float OverlapAmount1D(float a_min, float a_max, float b_min, float b_max) {
    return std::min(a_max, b_max) - std::max(a_min, b_min);
}

inline bool NearlyEqual(float a, float b, float eps = 0.01f) {
    return std::abs(a - b) <= eps;
}

inline Box2 MoveBox(Box2 box, Vec2f delta) {
    box.SetMinMax(box.Min() + delta, box.Max() + delta);
    return box;
}

struct VerticalMoveResult {
    bool hit = false;
    float dy = 0.0f;
};

float ResolveHorizontalMovement(
    const Box2& body,
    float dx,
    const TileWorldSystem& world) {
    if (dx == 0.0f) {
        return 0.0f;
    }

    Box2 query = MoveBox(body, Vec2f{ dx, 0.0f });
    query.UnionBox(body);

    float resolved_dx = dx;

    auto hits = world.querySolidTiles(query);

    for (const TileHit& hit_tile : hits) {
        const Box2& solid = hit_tile.aabb;

        // For horizontal movement, only care about current vertical overlap.
        if (!Overlap1DStrict(body.Min().y, body.Max().y, solid.Min().y, solid.Max().y)) {
            continue;
        }

        if (dx > 0.0f) {
            // Moving right. Tile must be currently to the right.
            if (body.Max().x <= solid.Min().x) {
                const float candidate_dx = solid.Min().x - body.Max().x;
                resolved_dx = std::min(resolved_dx, candidate_dx);
            }
        } else {
            // Moving left. Tile must be currently to the left.
            if (body.Min().x >= solid.Max().x) {
                const float candidate_dx = solid.Max().x - body.Min().x;
                resolved_dx = std::max(resolved_dx, candidate_dx);
            }
        }
    }

    return resolved_dx;
}

VerticalMoveResult ResolveUpMovement(
    const Box2& body,
    float dy,
    const TileWorldSystem& world) {
    VerticalMoveResult result;
    result.dy = dy;

    if (dy <= 0.0f) {
        return result;
    }

    Box2 query = MoveBox(body, Vec2f{ 0.0f, dy });
    query.UnionBox(body);

    auto hits = world.querySolidTiles(query);

    for (const TileHit& hit_tile : hits) {
        const Box2& solid = hit_tile.aabb;

        const bool x_overlap =
            body.Max().x - kStepOffset >= solid.Min().x &&
            body.Min().x + kStepOffset <= solid.Max().x;

        if (!x_overlap) {
            continue;
        }

        // Y-up:
        // player top crosses tile bottom.
        // player top = body.Max().y
        // tile bottom = solid.Min().y
        if (body.Max().y <= solid.Min().y &&
            body.Max().y + dy >= solid.Min().y) {
            const float candidate_dy = solid.Min().y - body.Max().y;

            result.hit = true;
            // dy is positive, choose the closest ceiling, i.e. smallest dy.
            result.dy = std::min(result.dy, candidate_dy);
        }
    }

    return result;
}

VerticalMoveResult ResolveDownMovement(
    const Box2& body,
    float dy,
    const TileWorldSystem& world) {
    VerticalMoveResult result;
    result.dy = dy;

    if (dy >= 0.0f) {
        return result;
    }

    Box2 query = MoveBox(body, Vec2f{ 0.0f, dy });
    query.UnionBox(body);

    auto hits = world.querySolidTiles(query);

    for (const TileHit& hit_tile : hits) {
        const Box2& solid = hit_tile.aabb;

        const float overlap_x = OverlapAmount1D(
            body.Min().x,
            body.Max().x,
            solid.Min().x,
            solid.Max().x);

        if (overlap_x <= kMinGroundSupport) {
            continue;
        }

        // Y-up:
        // player bottom crosses tile top.
        // player bottom = body.Min().y
        // tile top = solid.Max().y
        if (body.Min().y >= solid.Max().y &&
            body.Min().y + dy <= solid.Max().y) {
            const float candidate_dy = solid.Max().y - body.Min().y;

            result.hit = true;
            // dy is negative, choose the closest ground, i.e. largest dy.
            result.dy = std::max(result.dy, candidate_dy);
        }
    }

    return result;
}

bool CheckWallGrab(
    const Box2& body,
    float dy,
    const LegacyPlayerMotor&,
    const TileWorldSystem& world) {
    // Only used to START grabbing.
    // Existing grab state is handled by MoveVertical().
    if (dy >= 0.0f) {
        return false;
    }

    Box2 query = body;
    query.SetMinMax(
        query.Min() + Vec2f{ -kGrabEps, dy },
        query.Max() + Vec2f{ kGrabEps, 0.0f });

    auto hits = world.querySolidTiles(query);

    for (const TileHit& hit_tile : hits) {
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

void MoveHorizontal(
    TransformComponent& transform,
    const ColliderComponent& collider,
    const VelocityComponent& vel,
    const TileWorldSystem& world,
    float dt) {
    const float dx = vel.linear.x * dt;

    if (dx == 0.0f) {
        return;
    }

    const Box2 body = ComputeWorldAABB(transform, collider);
    const float resolved_dx = ResolveHorizontalMovement(body, dx, world);

    transform.Translate({ resolved_dx, 0.0f, 0.0f });
}

void MoveVertical(
    TransformComponent& transform,
    const ColliderComponent& collider,
    VelocityComponent& vel,
    LegacyPlayerMotor& motor,
    const TileWorldSystem& world,
    float dt) {
    Box2 body = ComputeWorldAABB(transform, collider);
    const float dy = vel.linear.y * dt;

    // Already grabbing: stay grabbing.
    // Only TryJump(), hurt, revive/reset should clear motor.grabbing.
    if (motor.grabbing && !motor.hurt) {
        vel.linear.y = 0.0f;
        return;
    }

    // Can only START grabbing while airborne and falling.
    // In your current motor, taking_jump means "grounded / can jump".
    const bool can_start_grab =
        !motor.taking_jump &&
        !motor.hurt &&
        dy < 0.0f;

    if (can_start_grab && CheckWallGrab(body, dy, motor, world)) {
        motor.grabbing = true;
        vel.linear.y = 0.0f;
        return;
    }

    if (dy < 0.0f) {
        VerticalMoveResult down = ResolveDownMovement(body, dy, world);

        if (down.hit) {
            transform.Translate({ 0.0f, down.dy, 0.0f });
            Land(motor, vel);
            return;
        }
    } else if (dy > 0.0f) {
        VerticalMoveResult up = ResolveUpMovement(body, dy, world);

        if (up.hit) {
            transform.Translate({ 0.0f, up.dy, 0.0f });
            vel.linear.y = 0.0f;
            return;
        }
    }

    transform.Translate({ 0.0f, dy, 0.0f });
    vel.linear.y += kGravity * dt;
}

void TryJump(VelocityComponent& vel,
             LegacyPlayerMotor& motor) {

    if (motor.grabbing) {
        vel.linear.y = kWallJumpForce;
        motor.taking_jump = false;
        motor.grabbing = false;
        return;
    }

    if (motor.taking_jump) {
        vel.linear.y = kJumpForce;
        motor.taking_jump = false;
        motor.grabbing = false;
        return;
    }
}

void UpdatePlayerState(VelocityComponent& vel, LegacyPlayerMotor& motor) {
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

}  // namespace

void PlayerController::onCreate() {
    const SceneQuery query(context().scene);

    animator_ = query.findChildByName("player_animator_node", entity());
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

void PlayerController::onUpdate(float dt) {
    const IGameInput& input = context().game_input;
    SceneQuery query(context().scene);

    const bool jump_pressed = input.isPressed("ui_up"_sid);
    int move_x = input.isPressed("ui_right"_sid) - input.isPressed("ui_left"_sid);
    if (motor_.grabbing) {
        move_x = 0;
    }

    auto transform = query.component<TransformComponent>(entity());
    auto collider = query.component<ColliderComponent>(entity());
    auto vel = query.component<VelocityComponent>(entity());
    DEV_ASSERT(transform && collider && vel);

    const TileWorldSystem* tile_world = query.system<TileWorldSystem>();
    DEV_ASSERT(tile_world);

    vel->linear.x = move_x * motor_.speed;

    // Jump is allowed to cancel grabbing.
    if (jump_pressed) {
        TryJump(*vel, motor_);
    }

    // Movement.
    MoveHorizontal(*transform, *collider, *vel, *tile_world, dt);
    MoveVertical(*transform, *collider, *vel, motor_, *tile_world, dt);

    // State + animation after movement/collision.
    UpdatePlayerState(*vel, motor_);
    updateAnimation(query);

    drawDebug();
}

void PlayerController::drawDebug() {
#if 0
    SceneQuery query(context().scene);
    const TileWorldSystem* tile_world = query.system<TileWorldSystem>();

    constexpr Vec4f kPlayerColor(0, 0, 1, 0.9f);
    constexpr Vec4f kTileColor(1, 0, 0, 0.9f);
    constexpr float tile_size = 1.0f;

    IDebugDrawService& debug_draw = context().engine_services.debugDraw();
    Box2 aabb = GetPlayerAABB(query);

    aabb = ExpandAABB(aabb, Vec2f(tile_size));
    auto hits = tile_world->querySolidTiles(aabb);

    for (const TileHit& hit : hits) {
        debug_draw.addBox2Frame(hit.aabb.Min(), hit.aabb.Max(), kTileColor, 0.05f);
    }
#endif
}

}  // namespace super_cave_boy
