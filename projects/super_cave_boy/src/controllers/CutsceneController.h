#pragma once
#include "cave/runtime/script/native/NativeScript.h"
#include "cave/runtime/game/StateMachine.h"

namespace super_cave_boy {

enum class CutsceneState : uint8_t {
    Inactive = 0,
    MoveToGuardian,
    Wait,
    MoveToPlayer,
    End,
    Count,
    Invalid = Count,
};

class CutsceneController final : public ::cave::NativeScript {
    using Entity = cave::ecs::Entity;

private:
    void start() override;

    void update(float dt) override;

    void onBodyEntered(Entity ent) override;

    void updateMove(float);

    cave::GameStateMachine<CutsceneState> m_state_machine;
    Entity m_camera;
    bool m_fired = false;

    float m_speed = 0.0f;
    float m_initial_x = 0.0f;
    float m_guardian_x = 0.0f;
};

}  // namespace super_cave_boy

