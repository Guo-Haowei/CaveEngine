#pragma once
#include "cave/runtime/game/StateMachine.h"

#include "EnemyControllerBase.h"

namespace super_cave_boy {

enum class BatState : uint8_t {
    Idle = 0,
    Move,
    Count,
    Invalid = Count,
};

class BatController : public EnemyControllerBase {
private:
    void start() override;
    void update(float dt) override;

    void updateIdle(float dt);
    void updateMove(float dt);

    bool canSeePlayer(cave::math::Vec2f bat_pos,
                      cave::math::Vec2f player_pos) const;

private:
    cave::GameStateMachine<BatState> m_state_machine;

    cave::math::Vec2f m_detect_range{ 5, 5 };
    float m_speed = 2.0f;
    float m_close_speed = 3.0f;
    float m_align_epsilon = 0.08f;
};

}  // namespace super_cave_boy
