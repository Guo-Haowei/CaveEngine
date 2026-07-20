#include "BatController.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/core/error/ErrorMacros.h"
#include "cave/runtime/display/ICanvas.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/MovementComponent.h"
#include "cave/runtime/ecs/components/SpriteAnimatorComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/scene/SceneRuntime.h"

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

bool BatController::shouldRecomputePath(TileCoord player_tile) const {
    const bool no_path = m_path_ctx.path.empty() && !m_path_ctx.recompute_timer.active();

    const bool target_changed = player_tile != m_path_ctx.goal_tile;

    return no_path ||
           target_changed ||
           m_path_ctx.recompute_timer.finished();
}

void BatController::updateMove(float dt) {
    const TileWorldSystem* tile_world = system<TileWorldSystem>();
    DEV_ASSERT(tile_world);

    auto transform = component<TransformComponent>();
    auto collider = component<ColliderComponent>();
    auto player_transform = query().component<TransformComponent>(m_player);

    DEV_ASSERT(transform && collider && player_transform);

    const Vec2f bat_pos = transform->translation().xy;
    const Vec2f player_pos = player_transform->translation().xy;

    const TileCoord start = TileWorldSystem::worldToTile(bat_pos);
    const TileCoord goal = TileWorldSystem::worldToTile(player_pos);

    if (!m_path_ctx.path.empty()) {
        auto& canvas = services().canvas();

        canvas.pushView(runtime().viewId());
        for (TileCoord coord : m_path_ctx.path) {
            canvas.addBox2Frame(
                Vec2f(coord.x, coord.y),
                Vec2f(coord.x + 1.0f, coord.y + 1.0f),
                0.02f);
        }
        canvas.popView();
    }

    m_path_ctx.recompute_timer.tick(dt);

    // Important: do not assign goal_tile before this check.
    // Otherwise target_changed will always be false.
    if (shouldRecomputePath(goal)) {
        auto path = tile_world->findPathAstar(start, goal);

        m_path_ctx.path = std::move(path);
        m_path_ctx.goal_tile = goal;
        m_path_ctx.recompute_timer.start();

        if (m_path_ctx.path.empty()) {
            m_path_ctx.index = -1;
        } else {
            // findPath normally includes the starting tile.
            // The bat is already there, so target the next tile.
            m_path_ctx.index =
                m_path_ctx.path.size() > 1 ? 1 : 0;
        }
    }

    // No path to follow.
    if (m_path_ctx.index < 0 ||
        m_path_ctx.index >=
            static_cast<int>(m_path_ctx.path.size())) {

        stopMoving();
        return;
    }

    // Skip waypoints that the bat has already reached.
    while (m_path_ctx.index < static_cast<int>(m_path_ctx.path.size())) {
        const TileCoord waypoint_tile = m_path_ctx.path[m_path_ctx.index];
        const Vec2f waypoint = TileWorldSystem::tileToWorld(waypoint_tile);
        const Vec2f delta = waypoint - bat_pos;
        const float distance_squared = delta.x * delta.x + delta.y * delta.y;

        constexpr float kWaypointEpsilon = 0.1f;  // world units

        if (distance_squared > kWaypointEpsilon * kWaypointEpsilon) {
            moveTowards(bat_pos, waypoint);
            return;
        }
        ++m_path_ctx.index;
    }

    // Reached the end of the current path.
    m_path_ctx.index = -1;
    stopMoving();
}

void BatController::moveTowards(Vec2f from, Vec2f to) {
    auto velocity = component<VelocityComponent>();
    DEV_ASSERT(velocity);

    const Vec2f delta = to - from;

    const float x_sign =
        SignWithDeadZone(delta.x, m_align_epsilon);

    const float y_sign =
        SignWithDeadZone(delta.y, m_align_epsilon);

    Vec2f desired_direction{
        x_sign,
        y_sign,
    };

    float speed = m_speed;

    // Preserve your existing slower movement when aligned
    // with one axis.
    if (desired_direction.x == 0.0f ||
        desired_direction.y == 0.0f) {

        speed = m_close_speed;
    }

    velocity->linear.x = desired_direction.x * speed;
    velocity->linear.y = desired_direction.y * speed;
}

void BatController::stopMoving() {
    auto velocity = component<VelocityComponent>();
    DEV_ASSERT(velocity);

    velocity->linear.x = 0.0f;
    velocity->linear.y = 0.0f;
}

}  // namespace super_cave_boy
