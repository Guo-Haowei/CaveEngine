#pragma once
#include "cave/core/math/Vec.h"
#include "cave/core/time/CountdownTimer.h"
#include "cave/runtime/game/StateMachine.h"
#include "cave/runtime/ecs/components/MovementComponent.h"
#include "cave/runtime/ecs/components/SpriteAnimatorComponent.h"
#include "cave/runtime/script/native/NativeScript.h"

#include "SuperCaveBoyDefines.h"

namespace super_cave_boy {

enum class PlayerState : uint8_t {
    Normal,
    Exiting,
    Count,
    Invalid = Count,
};

enum class PlayerNormalState : uint8_t {
    Idle,
    Walk,
    Air,
    Grab,
    Hurt,
};

struct PlayerHurtInfo {
    int damage{ 1 };
    cave::ecs::Entity entity;
};

class PlayerController final : public ::cave::NativeScript {
    using Entity = cave::ecs::Entity;

private:
    void start() override;
    void update(float dt) override;

    void updateNormal(float dt);

    void onEnterExiting();
    void updateExiting(float dt);

    void updateAnimation(cave::SpriteAnimatorComponent& animator);
    void updatePlayerState(cave::VelocityComponent& vel);

    void takeDamage(const PlayerHurtInfo& info);
    void bounceFromEnemy(float bounce_speed);

    void tryJump(cave::VelocityComponent& vel,
                 cave::MotorComponent& motor);

    void onBodyEntered(cave::ecs::Entity ent) override;
    void onBodyStay(cave::ecs::Entity ent) override;

    bool hurt() const { return m_hurt_timer.active(); }

    cave::GameStateMachine<PlayerState> m_state_machine;
    cave::CountdownTimer m_hurt_timer{ kPlayerHurtCountDown };

    PlayerNormalState m_state = PlayerNormalState::Air;

    Entity m_animator;
    bool m_block_input = false;
    int m_health;

    // @TODO: clean up
    bool m_taking_jump = false;
    bool m_landed = false;
    bool m_grabbing = false;
};

}  // namespace super_cave_boy
