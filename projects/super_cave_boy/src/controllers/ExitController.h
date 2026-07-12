#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/runtime/game/IGameModule.h"
#include "cave/runtime/game/StateMachine.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

enum ExitState {
    Inactive = 0,
    Active,
    Expired,
    Invisible,
    Count,
    Invalid = Count,
};

class ExitController final : public ::cave::NativeScript {
    using Entity = cave::ecs::Entity;

protected:
    void start() override;
    void update(float dt) override;

    void onBodyStay(Entity player) override;

    cave::GameStateMachine<ExitState> m_state_machine;
    bool m_fired = false;
};

}  // namespace super_cave_boy
