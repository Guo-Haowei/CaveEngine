#include "BatController.h"

#include "cave/core/error/ErrorMacros.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/MovementComponent.h"
#include "cave/runtime/ecs/components/SpriteAnimatorComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;
using ::cave::ecs::Entity;

namespace {

inline float SignWithDeadZone(float value, float eps) {
    if (value > eps) {
        return 1.0f;
    }

    if (value < -eps) {
        return -1.0f;
    }

    return 0.0f;
}

bool OverlapsSolidTiles(const Box2& aabb, const TileWorldSystem& world) {
    return !world.querySolidTiles(aabb).empty();
}

}  // namespace

bool BatController::canSeePlayer(const Vec2f& bat_pos,
                                 const Vec2f& player_pos) const {
    const float dx = std::abs(player_pos.x - bat_pos.x);
    const float dy = std::abs(player_pos.y - bat_pos.y);

    const bool close_x = dx <= detect_range_x_;
    const bool valid_y = dy <= detect_range_y_;

    return close_x && valid_y;
}

void BatController::onUpdate(float dt) {
    SceneQuery query(context().scene);

    if (!player_.IsValid()) {
        player_ = findPlayer(query);
    }

    switch (state_) {
        case BatState::Idle: {
            updateIdle(query);
        } break;
        case BatState::Move: {
            updateMove(query, dt);
        } break;
    }

    updateAnimation(query);
}

void BatController::updateIdle(SceneQuery& query) {
    auto transform = query.component<TransformComponent>(entity());
    auto player_transform = query.component<TransformComponent>(player_);

    DEV_ASSERT(transform && player_transform);

    const Vec2f bat_pos = transform->GetTranslation().xy;
    const Vec2f player_pos = player_transform->GetTranslation().xy;

    if (canSeePlayer(bat_pos, player_pos)) {
        state_ = BatState::Move;

        if (auto animator = query.component<SpriteAnimatorComponent>(animator_)) {
            animator->currentClip("fly");
        }
    }
}

void BatController::updateMove(SceneQuery& query, float) {
    auto transform = query.component<TransformComponent>(entity());
    auto collider = query.component<ColliderComponent>(entity());
    auto vel = query.component<VelocityComponent>(entity());
    auto player_transform = query.component<TransformComponent>(player_);

    const TileWorldSystem* tile_world = query.system<TileWorldSystem>();

    DEV_ASSERT(transform && collider && vel && player_transform && tile_world);

    const Vec2f bat_pos = transform->GetTranslation().xy;
    const Vec2f player_pos = player_transform->GetTranslation().xy;

    const float diff_x = bat_pos.x - player_pos.x;
    const float diff_y = bat_pos.y - player_pos.y;

    const float xsign = SignWithDeadZone(diff_x, align_epsilon_);
    const float ysign = SignWithDeadZone(diff_y, align_epsilon_);

    Vec2f desired_dir{
        -xsign,
        -ysign,
    };

    float speed = speed_;

    if (desired_dir.x == 0.0f || desired_dir.y == 0.0f) {
        speed = close_speed_;
    }

    vel->linear.x = desired_dir.x * speed;
    vel->linear.y = desired_dir.y * speed;
}

void BatController::updateAnimation(SceneQuery& query) {
    auto animator = query.component<SpriteAnimatorComponent>(animator_);

    if (!animator) {
        return;
    }

    switch (state_) {
        case BatState::Idle:
            animator->currentClip("idle");
            break;

        case BatState::Move:
            animator->currentClip("fly");
            break;
    }
}

}  // namespace super_cave_boy
