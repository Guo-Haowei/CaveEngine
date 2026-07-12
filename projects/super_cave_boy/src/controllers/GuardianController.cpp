#include "GuardianController.h"

#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/MovementComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"

namespace super_cave_boy {

using namespace ::cave;
using ::cave::ecs::Entity;

namespace {

constexpr float kFollowDuration = 2.0f;
constexpr float kLandedCooldown = 0.5f;
constexpr float kDropDelay = 0.35f;
constexpr float kFallSpeed = -7.0f;
constexpr float kRaiseSpeed = 5.0f;
constexpr float kAlignEpsilon = 0.4f;
constexpr float kWallDistance = 1.0f;

}  // namespace

GuardianController::GuardianController() noexcept {
    m_health = 3;
    m_health = 1;
}

void GuardianController::start() {
    EnemyControllerBase::start();

    auto* transform = component<TransformComponent>();
    DEV_ASSERT(transform);

    m_ground_y = transform->translation().y;
    m_hover_y = m_ground_y + m_rise_height;

    m_state_machine.addState(
        GuardianState::Inactive,
        {});

    m_state_machine.addState(
        GuardianState::Raising,
        {
            .update = [this](float dt) { updateRaising(dt); },
            //.on_enter = [this]() { enterRaising(); },
        });

    m_state_machine.addState(
        GuardianState::Falling,
        {
            .update = [this](float dt) { updateFalling(dt); },
            .on_enter = [this]() { enterFalling(); },
        });

    m_state_machine.addState(
        GuardianState::Follow,
        {
            .update = [this](float dt) { updateFollow(dt); },
            .on_enter = [this]() { enterFollow(); },
            .duration = kFollowDuration,
            .next = GuardianState::Wait,
        });

    m_state_machine.addState(
        GuardianState::Wait,
        {
            .on_enter = [this]() { enterWait(); },
            .duration = kDropDelay,
            .next = GuardianState::Falling,
        });

    m_state_machine.addState(
        GuardianState::Landed,
        {
            .on_enter = [this]() { enterLanded(); },
            .duration = kLandedCooldown,
            .next = GuardianState::Raising,
        });

    m_state_machine.switchTo(GuardianState::Inactive);

    m_begin_fight_listener = runtime().messageBus().listen(
        kGuardianBeginFight,
        [this](const Message&) {
            beginFight();
        });
}

void GuardianController::update(float dt) {
    m_state_machine.update(dt);
}

void GuardianController::takeDamageFromPlayer(int damage) {
    if (DEV_VERIFY(m_health > 0)) {
        m_health -= damage;
        if (alive()) {
            return;
        }

        auto& message_bus = runtime().messageBus();
        message_bus.emit(kGuardianDefeated, entity());
        message_bus.disconnect(m_begin_fight_listener);
        query().queueDestroy(entity());
    }
}

void GuardianController::beginFight() {
    if (DEV_VERIFY(m_state_machine.is(GuardianState::Inactive))) {
        m_state_machine.switchTo(GuardianState::Raising);
    }
}

void GuardianController::updateRaising(float) {
    auto* transform = component<TransformComponent>();
    auto* velocity = component<VelocityComponent>();

    DEV_ASSERT(transform && velocity);

    auto position = transform->translation();

    const float distance = m_hover_y - position.y;
    if (distance <= 0.01f) {
        position.y = m_hover_y;
        transform->setTranslation(position);

        velocity->linear.y = 0.0f;
        m_state_machine.switchTo(GuardianState::Follow);
        return;
    }

    velocity->linear.y = kRaiseSpeed;
}

void GuardianController::enterFollow() {
    auto* velocity = component<VelocityComponent>();
    auto* motor = component<MotorComponent>();

    DEV_ASSERT(velocity && motor);

    velocity->linear.y = 0.0f;
    motor->affected_by_gravity = false;

    playAnimation("move");
}

void GuardianController::updateFollow(float) {
    auto* guardian_transform = component<TransformComponent>();
    auto* player_transform = query().component<TransformComponent>(m_player);
    auto* velocity = component<VelocityComponent>();

    DEV_ASSERT(guardian_transform && player_transform && velocity);

    const float guardian_x = guardian_transform->translation().x;
    const float guardian_y = guardian_transform->translation().y;

    const float player_x = player_transform->translation().x;

    const float dx = player_x - guardian_x;

    if (math::abs(dx) <= kAlignEpsilon) {
        velocity->linear.x = 0.0f;
        m_state_machine.switchTo(GuardianState::Wait);
        return;
    }

    const float direction = math::sign(dx);
    velocity->linear.x = direction * m_follow_speed;

    const auto* tile_world = system<TileWorldSystem>();
    if (DEV_VERIFY(tile_world)) {
        const float front_x = guardian_x + direction * kWallDistance;
        const TileCoord tile = TileWorldSystem::worldToTile({ front_x, guardian_y });
        if (tile_world->isSolid(tile)) {
            velocity->linear.x = 0.0f;
        }
    }
}

void GuardianController::enterWait() {
    auto* velocity = component<VelocityComponent>();
    DEV_ASSERT(velocity);

    velocity->linear.x = 0.0f;
    velocity->linear.y = 0.0f;

    playAnimation("move");
}

void GuardianController::enterFalling() {
    auto* velocity = component<VelocityComponent>();
    DEV_ASSERT(velocity);

    velocity->linear.x = 0.0f;
    velocity->linear.y = 0.0f;

    playAnimation("move");
}

void GuardianController::updateFalling(float) {
    auto* velocity = component<VelocityComponent>();
    auto* contact = component<ContactComponent>();
    DEV_ASSERT(velocity && contact);

    velocity->linear.x = 0.0f;
    velocity->linear.y = kFallSpeed;

    if (contact->hit_down) {
        m_state_machine.switchTo(GuardianState::Landed);
    }
}

void GuardianController::enterLanded() {
    auto* velocity = component<VelocityComponent>();
    DEV_ASSERT(velocity);

    velocity->linear.x = 0.0f;
    velocity->linear.y = 0.0f;
    playAnimation("move");
}

}  // namespace super_cave_boy
