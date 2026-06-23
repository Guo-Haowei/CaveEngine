#pragma once
#include "EnemyControllerBase.h"

namespace super_cave_boy {

enum class SpiderState {
    Idle,
    PrepareAttack,
    Attack,
    Air,
    Wait,
};

class SpiderController : public EnemyControllerBase {
public:
    void onCreate() override;
    void onUpdate(float dt) override;

private:
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
    SpiderState state_ = SpiderState::Idle;

    float wait_timer_ = 0.0f;

    // Tile units, not old JS pixels.
    float detect_range_x_ = 6.0f;
    float detect_above_ = 3.0f;
    float detect_below_ = 1.0f;

    float attack_range_x_ = 6.0f;

    float jump_y_speed_ = 14.0f;
    float min_jump_x_speed_ = 4.0f;
    float max_jump_x_speed_ = 12.0f;
    float jump_x_distance_scale_ = 0.35f;

    float wait_duration_ = 1.0f;
};

}  // namespace super_cave_boy