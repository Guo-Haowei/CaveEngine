#pragma once
#include "cave/runtime/game/StateMachine.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

enum ExitState {
    NotTriggered = 0,
    Triggered,
    Expired,
    Hidden,
    Appearing,
    Count,
    Invalid = Count,
};

class ExitController final : public ::cave::NativeScript {
    using Entity = cave::ecs::Entity;

protected:
    void start() override;
    void update(float dt) override;

    void updateAppear(float dt);

    void enterActive();
    void exitActive();

    void onBodyStay(Entity player) override;

    cave::GameStateMachine<ExitState> m_state_machine;
    Entity m_sprite;

    bool m_fired = false;
};

}  // namespace super_cave_boy
