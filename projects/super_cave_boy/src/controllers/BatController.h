#pragma once
#include "EnemyControllerBase.h"

namespace super_cave_boy {

enum class BatState {
    Idle,
    Move,
};

class BatController : public EnemyControllerBase {
private:
    void onUpdate(cave::SceneContext& ctx, float dt) override;

    void updateIdle(cave::SceneQuery& query);
    void updateMove(cave::SceneQuery& query, float dt);

    bool canSeePlayer(const cave::math::Vec2f& bat_pos,
                      const cave::math::Vec2f& player_pos) const;

    void updateAnimation(cave::SceneQuery& query);

private:
    BatState state_ = BatState::Idle;

    float speed_ = 2.0f;
    float close_speed_ = 3.0f;

    float detect_range_x_ = 5.f;
    float detect_range_y_ = 4.5f;

    float align_epsilon_ = 0.08f;
};

}  // namespace super_cave_boy
