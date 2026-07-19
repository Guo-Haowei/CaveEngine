#pragma once
#include "cave/core/time/CountdownTimer.h"
#include "cave/runtime/game/StateMachine.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"

#include "EnemyControllerBase.h"

namespace super_cave_boy {

enum class BatState : uint8_t {
    Idle = 0,
    Move,
    Count,
    Invalid = Count,
};

class BatController : public EnemyControllerBase {
    static constexpr float kRecomputePathCooldown = 1.0f;

private:
    void start() override;
    void update(float dt) override;

    void updateIdle(float dt);
    void updateMove(float dt);

    bool canSeePlayer(cave::math::Vec2f bat_pos,
                      cave::math::Vec2f player_pos) const;

    bool shouldRecomputePath(cave::TileCoord player_tile) const;

    void moveTowards(cave::math::Vec2f from, cave::math::Vec2f to);
    void stopMoving();

private:
    cave::GameStateMachine<BatState> m_state_machine;

    struct PathContext {
        cave::Vector<cave::TileCoord> path;
        int index = -1;

        cave::TileCoord goal_tile{};
        cave::CountdownTimer recompute_timer;
    } m_path_ctx;

    cave::math::Vec2f m_detect_range{ 5, 5 };
    float m_speed = 2.0f;
    float m_close_speed = 3.0f;
    float m_align_epsilon = 0.08f;
};

}  // namespace super_cave_boy
