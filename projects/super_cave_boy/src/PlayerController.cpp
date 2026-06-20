#include "PlayerController.h"

#include "cave/core/error/ErrorMacros.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/SpriteAnimatorComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/display/IDebugDrawService.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/input/IGameInput.h"
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

constexpr float kGravity = -35.0f;  // world units / second^2, tune this
constexpr float kJumpForce = 14.0f;  // world units / second, tune this
constexpr float kWallJumpForce = 9.5f;
constexpr float kStepOffset = 0.05f;
constexpr float kGrabEps = 0.03f;
constexpr float kMinGroundSupport = 0.05f;  // tune; tile size is 1.0

bool IsLedgeTile(const TileWorldSystem& world, const TileHit& hit) {
    TileCoord above = hit.coord;
    above.y += 1;

    return !world.isSolid(above);
}

Box2 ComputeWorldAABB(const TransformComponent& transform,
                      const ColliderComponent& collider) {
    const Shape& shape = collider.shape();
    Vec2f translation = transform.GetTranslation().xy;

    return {
        translation - Vec2f(shape.data.half.xy),
        translation + Vec2f(shape.data.half.xy),
    };
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
    LegacyPlayerMotor& motor,
    const TileWorldSystem& world) {
    // Y-up: falling is negative.
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

        // Critical difference from old JS:
        // A single tile is grabbable only if its top is exposed.
        if (!IsLedgeTile(world, hit_tile)) {
            continue;
        }

        // Y-up:
        // player top crosses tile top while falling.
        const bool player_top_crosses_tile_top =
            body.Max().y >= solid.Max().y &&
            body.Max().y + dy <= solid.Max().y;

        if (!player_top_crosses_tile_top) {
            continue;
        }

        // Player is on left side of tile, touching tile's left edge.
        if (NearlyEqual(body.Max().x, solid.Min().x, kGrabEps)) {
            motor.grabbing = true;
            // motor.face = Direction::Right;
            return true;
        }

        // Player is on right side of tile, touching tile's right edge.
        if (NearlyEqual(body.Min().x, solid.Max().x, kGrabEps)) {
            motor.grabbing = true;
            // motor.face = Direction::Left;
            return true;
        }
    }

    return false;
}

void Land(LegacyPlayerMotor& motor) {
    if (motor.vspeed == 0.0f) {
        return;
    }

    motor.taking_jump = true;
    motor.grabbing = false;
    motor.vspeed = 0.0f;
}

void MoveHorizontal(
    TransformComponent& transform,
    const ColliderComponent& collider,
    LegacyPlayerMotor& motor,
    const TileWorldSystem& world,
    float dt) {
    const float dx = motor.hspeed * motor.speed * dt;

    if (dx == 0.0f) {
        motor.grabbing = false;
        return;
    }

    const Box2 body = ComputeWorldAABB(transform, collider);
    const float resolved_dx = ResolveHorizontalMovement(body, dx, world);

    transform.Translate({ resolved_dx, 0.0f, 0.0f });

    motor.grabbing = false;
}

void MoveVertical(
    TransformComponent& transform,
    const ColliderComponent& collider,
    LegacyPlayerMotor& motor,
    const TileWorldSystem& world,
    float dt) {
    Box2 body = ComputeWorldAABB(transform, collider);

    const float dy = motor.vspeed * dt;

    // Optional wall grab check before vertical move.
    motor.grabbing = CheckWallGrab(body, dy, motor, world);

    if (motor.grabbing && !motor.hurt) {
        motor.vspeed = 0.0f;
        return;
    }

    if (dy < 0.0f) {
        VerticalMoveResult down = ResolveDownMovement(body, dy, world);

        if (down.hit) {
            transform.Translate({ 0.0f, down.dy, 0.0f });
            Land(motor);
            return;
        }
    } else if (dy > 0.0f) {
        VerticalMoveResult up = ResolveUpMovement(body, dy, world);

        if (up.hit) {
            transform.Translate({ 0.0f, up.dy, 0.0f });
            motor.vspeed = 0.0f;
            return;
        }
    }

    transform.Translate({ 0.0f, dy, 0.0f });
    motor.vspeed += kGravity * dt;
}

void TryJump(LegacyPlayerMotor& motor, bool jump_pressed) {
    if (!jump_pressed) {
        return;
    }

    if (motor.grabbing) {
        motor.vspeed = kWallJumpForce;
        motor.taking_jump = false;
        motor.grabbing = false;
        return;
    }

    if (motor.taking_jump) {
        motor.vspeed = kJumpForce;
        motor.taking_jump = false;
        motor.grabbing = false;
        return;
    }
}

}  // namespace

void PlayerController::onCreate() {
    const SceneQuery query(context().scene);

    animator_ = query.findFirstByName("player_animator_node");
    // @TODO: set player position
}

void PlayerController::onUpdate(float dt) {
    const IGameInput& input = context().game_input;
    SceneQuery query(context().scene);

    const int move_x = (int)input.isPressed("ui_right"_sid) - (int)input.isPressed("ui_left"_sid);
    const bool jump_pressed = input.isPressed("ui_up"_sid);

    auto animator = query.component<SpriteAnimatorComponent>(animator_);
    DEV_ASSERT(animator);

    auto transform = query.component<TransformComponent>(entity());
    auto collider = query.component<ColliderComponent>(entity());

    // @TODO: later
    if (move_x == 0 && !jump_pressed) {
        animator->SetClip("idle");
    } else {
        animator->SetClip("walk");

        Vec4f rotation = move_x < 0 ? Vec4f{ 0.0f, 1.0f, 0.0f, 0.0f } : Vec4f{ 0.0f, 0.0f, 0.0f, 1.0f };
        transform->SetRotation(rotation);
    }

    motor_.hspeed = static_cast<float>(move_x);

    const TileWorldSystem* tile_world = query.system<TileWorldSystem>();
    DEV_ASSERT(tile_world);

    // Order close to old JS:
    // 1. input
    // 2. grab/jump
    // 3. vertical
    // 4. horizontal/state movement
    motor_.grabbing = CheckWallGrab(ComputeWorldAABB(*transform, *collider), motor_.vspeed * dt, motor_, *tile_world);
    TryJump(motor_, jump_pressed);
    MoveHorizontal(*transform, *collider, motor_, *tile_world, dt);
    MoveVertical(*transform, *collider, motor_, *tile_world, dt);

    //-------------------------------------
    // debug draw, ignore
    //-------------------------------------
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
}

}  // namespace super_cave_boy
