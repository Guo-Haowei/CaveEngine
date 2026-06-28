#include "SpiderController.h"

#include "cave/core/error/ErrorMacros.h"
#include "cave/core/math/Box.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/MovementComponent.h"
#include "cave/runtime/ecs/components/SpriteAnimatorComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/scene/MotorSystem.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;
using ::cave::ecs::Entity;

namespace {

Vec2f GetAABBCenter(const TransformComponent& transform,
                    const ColliderComponent& collider) {
    Box2 aabb = ComputeWorldAABB(transform, collider);
    return (aabb.min() + aabb.max()) * 0.5f;
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

void SpiderController::updateAir(SceneQuery& query, float) {
    const auto contact = query.component<ContactComponent>(entity());
    auto vel = query.component<VelocityComponent>(entity());
    DEV_ASSERT(contact && vel);

    if (contact->hit_down) {
        vel->linear.x = 0;
        vel->linear.y = 0;
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
