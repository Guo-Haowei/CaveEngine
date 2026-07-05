#pragma once

#include "EnemyControllerBase.h"

#include "Utility.h"

namespace super_cave_boy {

enum class SpiderState {
    Idle,
    PrepareAttack,
    Attack,
    Air,
    Wait,
};

class SpiderController : public EnemyControllerBase {
private:
    void onCreate(cave::SceneContext& ctx) override;
    void onUpdate(cave::SceneContext& ctx, float dt) override;

    void updateIdle(cave::SceneQuery& query, float dt);
    void enterAttack(cave::SceneQuery& query);
    void updateAir(cave::SceneQuery& query, float dt);
    void updateWait(float dt);

    void changeState(SpiderState state);

    bool canAttackPlayer(const cave::math::Vec2f& spider_pos,
                         const cave::math::Vec2f& player_pos) const;

    float computeJumpXSpeed(float distance_x) const;

    void updateAnimation(cave::SceneQuery& query);

private:
    SpiderState m_state = SpiderState::Idle;

    cave::math::Vec2f m_detect_range{ 5, 5 };

    CountdownTimer m_wait_timer{ 1.0f };

    float m_attack_range_x = 6.0f;

    float m_jump_y_speed = 14.0f;
    float m_min_jump_x_speed = 4.0f;
    float m_max_jump_x_speed = 12.0f;
    float m_jump_x_distance_scale = 0.35f;
};

}  // namespace super_cave_boy