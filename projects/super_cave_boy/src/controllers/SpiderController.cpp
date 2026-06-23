#include "SpiderController.h"

#include "cave/core/error/ErrorMacros.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/SpriteAnimatorComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/ecs/components/VelocityComponent.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;
using ::cave::ecs::Entity;

namespace {

constexpr float kGravity = -35.0f;
constexpr float kStepOffset = 0.05f;
constexpr float kMinGroundSupport = 0.05f;

inline Box2 MoveBox(Box2 box, Vec2f delta) {
    box.SetMinMax(box.Min() + delta, box.Max() + delta);
    return box;
}

inline bool Overlap1DStrict(float a_min, float a_max, float b_min, float b_max) {
    return a_max > b_min && a_min < b_max;
}

inline float OverlapAmount1D(float a_min, float a_max, float b_min, float b_max) {
    return std::min(a_max, b_max) - std::max(a_min, b_min);
}

Box2 ComputeWorldAABB(const TransformComponent& transform,
                      const ColliderComponent& collider) {
    const Shape& shape = collider.shape();

    // Use world translation because prefab/parent transform can offset the instance.
    Vec2f p = transform.GetTranslation().xy;

    return {
        p - Vec2f(shape.data.half.xy),
        p + Vec2f(shape.data.half.xy),
    };
}

Vec2f GetAABBCenter(const TransformComponent& transform,
                    const ColliderComponent& collider) {
    Box2 aabb = ComputeWorldAABB(transform, collider);
    return (aabb.Min() + aabb.Max()) * 0.5f;
}

struct VerticalMoveResult {
    bool hit = false;
    float dy = 0.0f;
};

struct TileMoveResult {
    bool hit_left = false;
    bool hit_right = false;
    bool hit_floor = false;
    bool hit_ceiling = false;

    Vec2f delta{ 0.0f, 0.0f };
};

float ResolveHorizontalMovement(const Box2& body,
                                float dx,
                                const TileWorldSystem& world) {
    if (dx == 0.0f) {
        return 0.0f;
    }

    Box2 query = MoveBox(body, { dx, 0.0f });
    query.UnionBox(body);

    float resolved_dx = dx;

    for (const TileHit& hit_tile : world.querySolidTiles(query)) {
        const Box2& solid = hit_tile.aabb;

        if (!Overlap1DStrict(body.Min().y, body.Max().y,
                             solid.Min().y, solid.Max().y)) {
            continue;
        }

        if (dx > 0.0f) {
            if (body.Max().x <= solid.Min().x) {
                const float candidate_dx = solid.Min().x - body.Max().x;
                resolved_dx = std::min(resolved_dx, candidate_dx);
            }
        } else {
            if (body.Min().x >= solid.Max().x) {
                const float candidate_dx = solid.Max().x - body.Min().x;
                resolved_dx = std::max(resolved_dx, candidate_dx);
            }
        }
    }

    return resolved_dx;
}

VerticalMoveResult ResolveUpMovement(const Box2& body,
                                     float dy,
                                     const TileWorldSystem& world) {
    VerticalMoveResult result;
    result.dy = dy;

    if (dy <= 0.0f) {
        return result;
    }

    Box2 query = MoveBox(body, { 0.0f, dy });
    query.UnionBox(body);

    for (const TileHit& hit_tile : world.querySolidTiles(query)) {
        const Box2& solid = hit_tile.aabb;

        const bool x_overlap =
            body.Max().x - kStepOffset >= solid.Min().x &&
            body.Min().x + kStepOffset <= solid.Max().x;

        if (!x_overlap) {
            continue;
        }

        if (body.Max().y <= solid.Min().y &&
            body.Max().y + dy >= solid.Min().y) {
            result.hit = true;
            result.dy = std::min(result.dy, solid.Min().y - body.Max().y);
        }
    }

    return result;
}

VerticalMoveResult ResolveDownMovement(const Box2& body,
                                       float dy,
                                       const TileWorldSystem& world) {
    VerticalMoveResult result;
    result.dy = dy;

    if (dy >= 0.0f) {
        return result;
    }

    Box2 query = MoveBox(body, { 0.0f, dy });
    query.UnionBox(body);

    for (const TileHit& hit_tile : world.querySolidTiles(query)) {
        const Box2& solid = hit_tile.aabb;

        const float overlap_x = OverlapAmount1D(
            body.Min().x,
            body.Max().x,
            solid.Min().x,
            solid.Max().x);

        if (overlap_x <= kMinGroundSupport) {
            continue;
        }

        if (body.Min().y >= solid.Max().y &&
            body.Min().y + dy <= solid.Max().y) {
            result.hit = true;
            result.dy = std::max(result.dy, solid.Max().y - body.Min().y);
        }
    }

    return result;
}

TileMoveResult MoveSpiderWithTileCollision(TransformComponent& transform,
                                           const ColliderComponent& collider,
                                           VelocityComponent& vel,
                                           const TileWorldSystem& world,
                                           float dt) {
    TileMoveResult result;

    // X first.
    const float requested_dx = vel.linear.x * dt;

    if (requested_dx != 0.0f) {
        const Box2 body = ComputeWorldAABB(transform, collider);
        const float dx = ResolveHorizontalMovement(body, requested_dx, world);

        if (dx != 0.0f) {
            transform.Translate({ dx, 0.0f, 0.0f });
        }

        result.delta.x = dx;

        if (requested_dx > 0.0f && dx < requested_dx) {
            result.hit_right = true;
            vel.linear.x = 0.0f;
        } else if (requested_dx < 0.0f && dx > requested_dx) {
            result.hit_left = true;
            vel.linear.x = 0.0f;
        }
    }

    // Y second.
    const float requested_dy = vel.linear.y * dt;

    if (requested_dy > 0.0f) {
        const Box2 body = ComputeWorldAABB(transform, collider);
        VerticalMoveResult up = ResolveUpMovement(body, requested_dy, world);

        if (up.dy != 0.0f) {
            transform.Translate({ 0.0f, up.dy, 0.0f });
        }

        result.delta.y = up.dy;

        if (up.hit) {
            result.hit_ceiling = true;
            vel.linear.y = 0.0f;
        } else {
            vel.linear.y += kGravity * dt;
        }

        return result;
    }

    if (requested_dy < 0.0f) {
        const Box2 body = ComputeWorldAABB(transform, collider);
        VerticalMoveResult down = ResolveDownMovement(body, requested_dy, world);

        if (down.dy != 0.0f) {
            transform.Translate({ 0.0f, down.dy, 0.0f });
        }

        result.delta.y = down.dy;

        if (down.hit) {
            result.hit_floor = true;
            vel.linear.y = 0.0f;
        } else {
            vel.linear.y += kGravity * dt;
        }

        return result;
    }

    // Even if current vertical velocity is zero, gravity should start pulling down.
    vel.linear.y += kGravity * dt;
    return result;
}

}  // namespace

void SpiderController::onCreate() {
    EnemyControllerBase::onCreate();
    changeState(SpiderState::Idle);
}

void SpiderController::onUpdate(float dt) {
    SceneQuery query(context().scene);

    if (!player_.IsValid()) {
        player_ = findPlayer(query);
    }

    switch (state_) {
        case SpiderState::Idle:
            updateIdle(query, dt);
            break;

        case SpiderState::Attack:
            enterAttack(query);
            break;

        case SpiderState::Air:
            updateAir(query, dt);
            break;

        case SpiderState::Wait:
            updateWait(dt);
            break;
    }

    updateAnimation(query);
}

void SpiderController::changeState(SpiderState state) {
    if (state_ == state) {
        return;
    }

    state_ = state;

    if (state_ == SpiderState::Wait) {
        wait_timer_ = wait_duration_;
    }
}

bool SpiderController::canAttackPlayer(const Vec2f& spider_pos,
                                       const Vec2f& player_pos) const {
    const float dx = std::abs(player_pos.x - spider_pos.x);
    const float dy = player_pos.y - spider_pos.y;

    const bool close_x = dx <= detect_range_x_;

    // Y-up convention:
    // allow player somewhat above spider and somewhat below spider.
    const bool valid_y =
        dy <= detect_above_ &&
        dy >= -detect_below_;

    return close_x && valid_y;
}

float SpiderController::computeJumpXSpeed(float distance_x) const {
    const float speed = distance_x * jump_x_distance_scale_ + min_jump_x_speed_;
    return std::clamp(speed, min_jump_x_speed_, max_jump_x_speed_);
}

void SpiderController::updateIdle(SceneQuery& query, float) {
    auto transform = query.component<TransformComponent>(entity());
    auto collider = query.component<ColliderComponent>(entity());

    auto player_transform = query.component<TransformComponent>(player_);
    auto player_collider = query.component<ColliderComponent>(player_);

    DEV_ASSERT(transform && collider && player_transform && player_collider);

    const Vec2f spider_pos = GetAABBCenter(*transform, *collider);
    const Vec2f player_pos = GetAABBCenter(*player_transform, *player_collider);

    if (canAttackPlayer(spider_pos, player_pos)) {
        changeState(SpiderState::Attack);
    }
}

void SpiderController::enterAttack(SceneQuery& query) {
    auto transform = query.component<TransformComponent>(entity());
    auto collider = query.component<ColliderComponent>(entity());
    auto vel = query.component<VelocityComponent>(entity());

    auto player_transform = query.component<TransformComponent>(player_);
    auto player_collider = query.component<ColliderComponent>(player_);

    DEV_ASSERT(transform && collider && vel && player_transform && player_collider);

    const Vec2f spider_pos = GetAABBCenter(*transform, *collider);
    const Vec2f player_pos = GetAABBCenter(*player_transform, *player_collider);

    const float dx = player_pos.x - spider_pos.x;
    const float abs_dx = std::abs(dx);

    if (abs_dx > attack_range_x_) {
        changeState(SpiderState::Idle);
        return;
    }

    const float dir_x = dx >= 0.0f ? 1.0f : -1.0f;

    vel->linear.x = dir_x * computeJumpXSpeed(abs_dx);
    vel->linear.y = jump_y_speed_;

    changeState(SpiderState::Air);
}

void SpiderController::updateAir(SceneQuery& query, float dt) {
    auto transform = query.component<TransformComponent>(entity());
    auto collider = query.component<ColliderComponent>(entity());
    auto vel = query.component<VelocityComponent>(entity());

    const TileWorldSystem* tile_world = query.system<TileWorldSystem>();

    DEV_ASSERT(transform && collider && vel && tile_world);

    TileMoveResult move = MoveSpiderWithTileCollision(
        *transform,
        *collider,
        *vel,
        *tile_world,
        dt);

    if (move.hit_floor) {
        vel->linear.x = 0.0f;
        vel->linear.y = 0.0f;
        changeState(SpiderState::Wait);
    }
}

void SpiderController::updateWait(float dt) {
    wait_timer_ -= dt;

    if (wait_timer_ <= 0.0f) {
        wait_timer_ = 0.0f;
        changeState(SpiderState::Idle);
    }
}

void SpiderController::updateAnimation(SceneQuery& query) {
    auto animator = query.component<SpriteAnimatorComponent>(animator_);

    if (!animator) {
        return;
    }

    switch (state_) {
        case SpiderState::Idle: {
            animator->currentClip("idle");
        } break;
        case SpiderState::PrepareAttack: {
            animator->currentClip("prepare_attack");
        } break;
        case SpiderState::Attack:
        case SpiderState::Air: {
            animator->currentClip("air");
        } break;
        case SpiderState::Wait: {
            animator->currentClip("idle");
        } break;
    }
}

}  // namespace super_cave_boy
