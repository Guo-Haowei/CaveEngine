#include "ExitController.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/runtime/scene/ISceneTransitionRequests.h"
#include "cave/runtime/scene/SceneQuery.h"

#include "SuperCaveBoyDefines.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;

void ExitController::start() {
    auto on_enter = [this]() {
        runtime().messageBus().emit(kPlayerLeave, entity());
    };

    auto on_exit = [this]() {
        auto it = params().find("level");
        if (it != params().end()) {
            std::string_view level = it->second.asString();
            if (DEV_VERIFY(transition())) {
                transition()->requestSceneChange(std::format("@res://scenes/{}.scene", level));
            }
        }
    };

    m_state_machine.addState(
        ExitState::Active,
        {
            .update = nullptr,
            .on_enter = std::move(on_enter),
            .on_exit = std::move(on_exit),
            .duration = kExitAnimationDuration,
            .next = ExitState::Inactive,
        });

    m_state_machine.switchTo(ExitState::Inactive);
}

void ExitController::update(float dt) {
    m_state_machine.update(dt);
}

void ExitController::onBodyStay(Entity player) {
#if USING(ENABLE_ASSERT)
    auto* player_collider = query().component<ColliderComponent>(player);
    DEV_ASSERT(player_collider);
    DEV_ASSERT(IsPlayer(*player_collider));
#endif

    if (!m_triggered) {
        m_state_machine.switchTo(ExitState::Active);
        m_triggered = true;
    }
}

}  // namespace super_cave_boy
