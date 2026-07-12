#include "BatController.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/core/error/ErrorMacros.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/MovementComponent.h"
#include "cave/runtime/ecs/components/SpriteAnimatorComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/scene/SceneRuntime.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;
using ::cave::ecs::Entity;

namespace {

float SignWithDeadZone(float value, float eps) {
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

bool BatController::canSeePlayer(Vec2f bat_pos, Vec2f player_pos) const {
    const float dx = std::abs(player_pos.x - bat_pos.x);
    const float dy = std::abs(player_pos.y - bat_pos.y);

    const bool close_x = dx <= m_detect_range.x;
    const bool valid_y = dy <= m_detect_range.y;

    return close_x && valid_y;
}

void BatController::start() {
    EnemyControllerBase::start();

    m_state_machine.addState(
        BatState::Idle,
        {
            .update = std::bind_front(&BatController::updateIdle, this),
            .on_enter = [this]() { playAnimation("idle"); },
        });

    m_state_machine.addState(
        BatState::Move,
        {
            .update = std::bind_front(&BatController::updateMove, this),
            .on_enter = [this]() { playAnimation("fly"); },
        });

    m_state_machine.switchTo(BatState::Idle);
}

void BatController::update(float dt) {
    m_state_machine.update(dt);
}

void BatController::updateIdle(float) {
    auto transform = component<TransformComponent>();
    auto player_transform = query().component<TransformComponent>(m_player);

    DEV_ASSERT(transform && player_transform);

    const Vec2f bat_pos = transform->translation().xy;
    const Vec2f player_pos = player_transform->translation().xy;

    if (canSeePlayer(bat_pos, player_pos)) {
        m_state_machine.switchTo(BatState::Move);
    }
}

void BatController::updateMove(float) {
    const TileWorldSystem* tile_world = system<TileWorldSystem>();
    DEV_ASSERT(tile_world);

    auto transform = component<TransformComponent>();
    auto collider = component<ColliderComponent>();
    auto vel = component<VelocityComponent>();
    auto player_transform = query().component<TransformComponent>(m_player);

    DEV_ASSERT(transform && collider && vel && player_transform);

    const Vec2f bat_pos = transform->translation().xy;
    const Vec2f player_pos = player_transform->translation().xy;

    const float diff_x = bat_pos.x - player_pos.x;
    const float diff_y = bat_pos.y - player_pos.y;

    const float xsign = SignWithDeadZone(diff_x, m_align_epsilon);
    const float ysign = SignWithDeadZone(diff_y, m_align_epsilon);

    Vec2f desired_dir{
        -xsign,
        -ysign,
    };

    float speed = m_speed;

    if (desired_dir.x == 0.0f || desired_dir.y == 0.0f) {
        speed = m_close_speed;
    }

    vel->linear.x = desired_dir.x * speed;
    vel->linear.y = desired_dir.y * speed;
}

}  // namespace super_cave_boy
