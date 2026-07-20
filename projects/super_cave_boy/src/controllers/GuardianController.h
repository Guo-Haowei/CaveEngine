#pragma once
#include "cave/core/time/CountdownTimer.h"
#include "cave/runtime/game/StateMachine.h"

#include "EnemyControllerBase.h"
#include "SuperCaveBoyDefines.h"

namespace super_cave_boy {

enum class GuardianState : uint8_t {
    Inactive = 0,
    Raising,
    Follow,
    Wait,
    Falling,
    Landed,
    Defeated,

    Count,
    Invalid = Count,
};

class GuardianController final : public EnemyControllerBase {
public:
    GuardianController() noexcept;

private:
    void start() override;

    void update(float dt) override;

    void takeDamage(int damage) override;

    void enterFollow();
    void updateFollow(float dt);

    void enterWait();

    void updateRaising(float dt);

    void enterFalling();
    void updateFalling(float dt);

    void enterLanded();

    void beginFight();

private:
    cave::GameStateMachine<GuardianState> m_state_machine;

    cave::ListenerId m_begin_fight_listener = 0;
    cave::ListenerId m_awake_listener = 0;

    cave::CountdownTimer m_hurt_timer{ 1.f };

    float m_ground_y = 0.0f;
    float m_hover_y = 0.0f;

    float m_rise_height = 4.0f;
    float m_follow_speed = 4.0f;
};

}  // namespace super_cave_boy