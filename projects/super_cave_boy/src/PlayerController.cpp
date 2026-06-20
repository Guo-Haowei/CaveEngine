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

inline bool Overlap1D(float a_min, float a_max, float b_min, float b_max) {
    return a_max >= b_min && a_min <= b_max;
}

inline bool NearlyEqual(float a, float b, float eps = 0.01f) {
    return std::abs(a - b) <= eps;
}

inline Box2 MoveBox(Box2 box, Vec2f delta) {
    box.SetMinMax(box.Min() + delta, box.Max() + delta);
    return box;
}

inline Box2 UnionBox(const Box2& a, const Box2& b) {
    return Box2{
        math::min(a.Min(), b.Min()),
        math::max(a.Max(), b.Max())
    };
}

float ResolveHorizontalMovement(
    const Box2& body,
    float dx,
    const TileWorldSystem& world) {
    if (dx == 0.0f) {
        return 0.0f;
    }

    const Box2 target = MoveBox(body, Vec2f{ dx, 0.0f });
    const Box2 query = UnionBox(body, target);

    float resolved_dx = dx;

    auto hits = world.querySolidTiles(query);

    for (const TileHit& hit_tile : hits) {
        const Box2& solid = hit_tile.aabb;

        // For horizontal movement, only care about current vertical overlap.
        if (!Overlap1D(body.Min().y, body.Max().y, solid.Min().y, solid.Max().y)) {
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

void MoveHorizontal(
    TransformComponent& transform,
    const ColliderComponent& collider,
    LegacyPlayerMotor& motor,
    const TileWorldSystem& world) {
    const float dx = motor.hspeed * motor.speed;

    if (dx == 0.0f) {
        motor.grabbing = false;
        return;
    }

    const Box2 body = ComputeWorldAABB(transform, collider);

    const float resolved_dx = ResolveHorizontalMovement(body, dx, world);

    transform.Translate({ resolved_dx, 0.0f, 0.0f });

    motor.grabbing = false;
}

}  // namespace

void PlayerController::onCreate() {
    const SceneQuery query(context().scene);

    animator_ = query.findFirstByName("player_animator_node");
}

void PlayerController::onDestroy() {
}

void PlayerController::onUpdate(float dt) {
    const IGameInput& input = context().game_input;
    SceneQuery query(context().scene);

    const int move_x = (int)input.isPressed("ui_right"_sid) - (int)input.isPressed("ui_left"_sid);
    int move_y = (int)input.isPressed("ui_up"_sid) - (int)input.isPressed("ui_down"_sid);
    move_y = 0;

    auto animator = query.component<SpriteAnimatorComponent>(animator_);
    DEV_ASSERT(animator);

    auto transform = query.component<TransformComponent>(entity());
    auto collider = query.component<ColliderComponent>(entity());

    // @TODO: later
    if (move_x == 0 && move_y == 0) {
        animator->SetClip("idle");
    } else {
        animator->SetClip("walk");

        // const float x_speed = 4.0f;
        // const float dx = x_speed * dt * move_x;
        // const float dy = x_speed * dt * move_y;

        // @TODO: deprecate
        // transform->IncreaseTranslation(Vec3f(dx, dy, 0.0f));

        Vec4f rotation = move_x < 0 ? Vec4f{ 0.0f, 1.0f, 0.0f, 0.0f } : Vec4f{ 0.0f, 0.0f, 0.0f, 1.0f };
        transform->SetRotation(rotation);
    }

    motor_.hspeed = move_x * dt;

    const TileWorldSystem* tile_world = query.system<TileWorldSystem>();
    MoveHorizontal(*transform, *collider, motor_, *tile_world);

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
