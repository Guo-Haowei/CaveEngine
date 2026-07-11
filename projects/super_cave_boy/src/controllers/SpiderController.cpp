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

void SpiderController::start(SceneContext& ctx) {
    EnemyControllerBase::start(ctx);

    m_state_machine.addState(
        SpiderState::Idle,
        {
            .update = std::bind_front(&SpiderController::updateIdle, this),
            .onEnter = [this](SceneContext& ctx) {
                EnemyControllerBase::playAnimation(ctx, "idle");
            },
        });
    m_state_machine.addState(
        SpiderState::PrepareAttack,
        {
            .update = std::bind_front(&SpiderController::updateIdle, this),
            .onEnter = [this](SceneContext& ctx) { playAnimation(ctx, "prepare_attack"); },
        });
    m_state_machine.addState(
        SpiderState::Attack,
        {
            .update = [this](SceneContext& ctx, float) { enterAttack(ctx); },
            .onEnter = [this](SceneContext& ctx) { playAnimation(ctx, "air"); },
        });
    m_state_machine.addState(
        SpiderState::Air,
        {
            .update = std::bind_front(&SpiderController::updateAir, this),
            .onEnter = [this](SceneContext& ctx) { playAnimation(ctx, "air"); },
        });
    m_state_machine.addState(
        SpiderState::Wait,
        {
            .update = std::bind_front(&SpiderController::updateWait, this),
            .onEnter = [this](SceneContext& ctx) { playAnimation(ctx, "idle"); m_wait_timer.start(); },
        });

    m_state_machine.switchTo(ctx, SpiderState::Idle);
}

void SpiderController::update(SceneContext& ctx, float dt) {
    m_state_machine.update(ctx, dt);
}

bool SpiderController::canAttackPlayer(const Vec2f& spider_pos,
                                       const Vec2f& player_pos) const {
    const float dx = std::abs(player_pos.x - spider_pos.x);
    const float dy = std::abs(player_pos.y - spider_pos.y);

    const bool close_x = dx <= m_detect_range.x;
    const bool close_y = dy <= m_detect_range.y;
    return close_x && close_y;
}

float SpiderController::computeJumpXSpeed(float distance_x) const {
    const float speed = distance_x * m_jump_x_distance_scale + m_min_jump_x_speed;
    return std::clamp(speed, m_min_jump_x_speed, m_max_jump_x_speed);
}

void SpiderController::updateIdle(SceneContext& ctx, float) {
    SceneQuery& query = ctx.query;
    auto transform = query.component<TransformComponent>(entity());
    auto collider = query.component<ColliderComponent>(entity());

    auto player_transform = query.component<TransformComponent>(m_player);
    auto player_collider = query.component<ColliderComponent>(m_player);

    DEV_ASSERT(transform && collider && player_transform && player_collider);

    const Vec2f spider_pos = GetAABBCenter(*transform, *collider);
    const Vec2f player_pos = GetAABBCenter(*player_transform, *player_collider);

    if (canAttackPlayer(spider_pos, player_pos)) {
        m_state_machine.switchTo(ctx, SpiderState::Attack);
    }
}

void SpiderController::enterAttack(SceneContext& ctx) {
    SceneQuery& query = ctx.query;
    auto transform = query.component<TransformComponent>(entity());
    auto collider = query.component<ColliderComponent>(entity());
    auto vel = query.component<VelocityComponent>(entity());

    auto player_transform = query.component<TransformComponent>(m_player);
    auto player_collider = query.component<ColliderComponent>(m_player);

    DEV_ASSERT(transform && collider && vel && player_transform && player_collider);

    const Vec2f spider_pos = GetAABBCenter(*transform, *collider);
    const Vec2f player_pos = GetAABBCenter(*player_transform, *player_collider);

    const float dx = player_pos.x - spider_pos.x;
    const float abs_dx = std::abs(dx);

    if (abs_dx > m_attack_range_x) {
        m_state_machine.switchTo(ctx, SpiderState::Idle);
        return;
    }

    const float dir_x = dx >= 0.0f ? 1.0f : -1.0f;

    vel->linear.x = dir_x * computeJumpXSpeed(abs_dx);
    vel->linear.y = m_jump_y_speed;

    m_state_machine.switchTo(ctx, SpiderState::Air);
}

void SpiderController::updateAir(SceneContext& ctx, float) {
    SceneQuery& query = ctx.query;
    const auto contact = query.component<ContactComponent>(entity());
    auto vel = query.component<VelocityComponent>(entity());
    DEV_ASSERT(contact && vel);

    if (contact->hit_down) {
        vel->linear.x = 0;
        vel->linear.y = 0;
        m_state_machine.switchTo(ctx, SpiderState::Wait);
    }
}

void SpiderController::updateWait(SceneContext& ctx, float dt) {
    m_wait_timer.tick(dt);
    if (m_wait_timer.finished()) {
        m_state_machine.switchTo(ctx, SpiderState::Idle);
    }
}

}  // namespace super_cave_boy
