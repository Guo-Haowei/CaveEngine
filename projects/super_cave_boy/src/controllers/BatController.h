#pragma once
#include "EnemyControllerBase.h"

namespace super_cave_boy {

enum class BatState {
    Idle,
    Move,
};

class BatController : public EnemyControllerBase {
private:
    void update(cave::SceneContext& ctx, float dt) override;

    void updateIdle(cave::SceneQuery& query);
    void updateMove(cave::SceneQuery& query, float dt);

    bool canSeePlayer(const cave::math::Vec2f& bat_pos,
                      const cave::math::Vec2f& player_pos) const;

    void updateAnimation(cave::SceneQuery& query);

private:
    BatState m_state = BatState::Idle;

    cave::math::Vec2f m_detect_range{ 5, 5 };
    float m_speed = 2.0f;
    float m_close_speed = 3.0f;
    float m_align_epsilon = 0.08f;
};

}  // namespace super_cave_boy
