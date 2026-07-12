#pragma once
#include "cave/runtime/game/StateMachine.h"

#include "EnemyControllerBase.h"
#include "SuperCaveBoyDefines.h"

namespace super_cave_boy {

enum class SpiderState : uint8_t {
    Idle = 0,
    PrepareAttack,
    Attack,
    Air,
    Wait,
    Count,
    Invalid = Count,
};

class SpiderController : public EnemyControllerBase {
private:
    void start() override;
    void update(float dt) override;

    void updateIdle(float dt);
    void enterAttack();
    void updateAir(float dt);
    void updateWait(float dt);

    bool canAttackPlayer(cave::math::Vec2f spider_pos,
                         cave::math::Vec2f player_pos) const;

    float computeJumpXSpeed(float distance_x) const;

private:
    cave::GameStateMachine<SpiderState> m_state_machine;

    cave::math::Vec2f m_detect_range{ 5, 5 };

    float m_attack_range_x = 6.0f;

    float m_jump_y_speed = 14.0f;
    float m_min_jump_x_speed = 4.0f;
    float m_max_jump_x_speed = 12.0f;
    float m_jump_x_distance_scale = 0.35f;
};

}  // namespace super_cave_boy