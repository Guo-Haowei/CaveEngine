#include "ExitController.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/runtime/ecs/components/SpriteRendererComponent.h"
#include "cave/runtime/scene/ISceneTransitionRequests.h"
#include "cave/runtime/scene/SceneQuery.h"

#include "SuperCaveBoyDefines.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;

namespace {
constexpr float kAppearingAnimationDuration = 0.5f;
}  // namespace

void ExitController::start() {
    m_sprite = query().findChildByName("sprite", entity());

    m_state_machine.addState(
        ExitState::Triggered,
        {
            .update = nullptr,
            .on_enter = std::bind_front(&ExitController::enterActive, this),
            .on_exit = std::bind_front(&ExitController::exitActive, this),
            .duration = kExitAnimationDuration,
            .next = ExitState::NotTriggered,
        });

    m_state_machine.addState(
        ExitState::Hidden,
        {
            .update = nullptr,
            .on_enter = [this]() {
                auto* sprite = query().component<SpriteRendererComponent>(m_sprite);
                if (DEV_VERIFY(sprite)) {
                    sprite->tintColor().a = 0.0f;
                }
            },
        });

    m_state_machine.addState(
        ExitState::Appearing,
        {
            .update = std::bind_front(&ExitController::updateAppear, this),
        });

    auto it = params().find("initial_state");
    if (it != params().end() && it->second.asString() == "hidden") {
        m_state_machine.switchTo(ExitState::Hidden);
    } else {
        m_state_machine.switchTo(ExitState::NotTriggered);
    }

    runtime().messageBus().listen(kGuardianDefeated, [this](const Message&) {
        if (DEV_VERIFY(m_state_machine.is(ExitState::Hidden))) {
            m_state_machine.switchTo(ExitState::Appearing);
        }
    });
}

void ExitController::update(float dt) {
    m_state_machine.update(dt);
}

void ExitController::updateAppear(float) {
    auto* sprite = query().component<SpriteRendererComponent>(m_sprite);
    float ratio = m_state_machine.stateTime() / kAppearingAnimationDuration;
    ratio = math::clamp(ratio, 0.0f, 1.0f);
    if (DEV_VERIFY(sprite)) {
        sprite->tintColor().a = math::clamp(ratio, 0.0f, 1.0f);
    }
    if (ratio >= 1.0f) {
        m_state_machine.switchTo(ExitState::NotTriggered);
    }
}

void ExitController::enterActive() {
    runtime().messageBus().emit(kPlayerLeave, entity());
}

void ExitController::exitActive() {
    auto it = params().find("level");
    if (it != params().end()) {
        std::string_view level = it->second.asString();
        if (DEV_VERIFY(transition())) {
            transition()->requestSceneChange(std::format("@res://scenes/{}.scene", level));
        }
    }
}

void ExitController::onBodyStay(Entity player) {
    if (m_state_machine.is(ExitState::NotTriggered) && !m_fired) {
#if USING(ENABLE_ASSERT)
        auto* player_collider = query().component<ColliderComponent>(player);
        DEV_ASSERT(player_collider);
        DEV_ASSERT(IsPlayer(*player_collider));
#else
        unused(player)
#endif
        m_state_machine.switchTo(ExitState::Triggered);
        m_fired = true;
    }
}

}  // namespace super_cave_boy
