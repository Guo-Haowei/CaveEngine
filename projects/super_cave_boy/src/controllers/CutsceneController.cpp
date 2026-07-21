#include "CutsceneController.h"

#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"

#include "SuperCaveBoyDefines.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;

namespace {
constexpr float kMoveDuration = 1.8f;
constexpr float kWaitDuration = 1.2f;
constexpr float kMoveSpeed = 6.0f;
}  // namespace

void CutsceneController::start() {
    m_camera = query().findFirstByName("GameCamera");

    m_initial_x = component<TransformComponent>()->translation().x;
    auto guardian = query().findFirstByName("Guardian");
    if (DEV_VERIFY(guardian.valid())) {
        auto* transform = query().component<TransformComponent>(guardian);
        DEV_ASSERT(transform);
        m_guardian_x = transform->translation().x;
    }

    m_state_machine.addState(
        CutsceneState::MoveToGuardian,
        {
            .update = std::bind_front(&CutsceneController::updateMove, this),
            .duration = kMoveDuration,
            .next = CutsceneState::Wait,
        });

    m_state_machine.addState(
        CutsceneState::Wait,
        {
            .on_enter = [this]() { message().emit(kGuardianAwakeID, entity()); },
            .on_exit = [this]() { message().emit(kGuardianBeginFightID, entity()); },
            .duration = kWaitDuration,
            .next = CutsceneState::MoveToPlayer,
        });

    m_state_machine.addState(
        CutsceneState::MoveToPlayer,
        {
            .update = std::bind_front(&CutsceneController::updateMove, this),
            .on_enter = [this]() { m_speed = -kMoveSpeed; },
            .duration = kMoveDuration,
            .next = CutsceneState::End,
        });

    m_state_machine.addState(
        CutsceneState::End,
        {
            .on_enter = [this]() { message().emit(kCutsceneEndID, entity()); },
        });

    m_state_machine.switchTo(CutsceneState::Inactive);

    m_speed = kMoveSpeed;
}

void CutsceneController::update(float dt) {
    m_state_machine.update(dt);
}

void CutsceneController::updateMove(float dt) {
    auto* transform = component<TransformComponent>();
    if (DEV_VERIFY(transform)) {
        Vec3f translation = transform->translation();
        const float x = translation.x + m_speed * dt;
        translation.x = math::clamp(x, m_initial_x, m_guardian_x);
        transform->setTranslation(translation);
    }
}

void CutsceneController::onBodyEntered(Entity player) {
    if (!m_fired) {
#if USING(ENABLE_ASSERT)
        auto* player_collider = query().component<ColliderComponent>(player);
        DEV_ASSERT(player_collider);
        DEV_ASSERT(IsPlayer(*player_collider));
#endif
        message().emit(kCutsceneStartID, entity());
        m_state_machine.switchTo(CutsceneState::MoveToGuardian);
        m_fired = true;
    }
}

}  // namespace super_cave_boy
