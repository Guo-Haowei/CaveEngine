#pragma once
#include "cave/core/math/Vector.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

enum class BatState {
    Idle,
    Move,
};

class BatController : public cave::NativeScript {
public:
    void onCreate() override;
    void onUpdate(float dt) override;

private:
    cave::ecs::Entity findPlayer(cave::SceneQuery& query) const;

    void updateIdle(cave::SceneQuery& query);
    void updateMove(cave::SceneQuery& query, float dt);

    bool canSeePlayer(const cave::math::Vec2f& bat_pos,
                      const cave::math::Vec2f& player_pos) const;

    void updateAnimation(cave::SceneQuery& query);

private:
    BatState state_ = BatState::Idle;

    cave::ecs::Entity player_{};
    cave::ecs::Entity animator_{};

    float speed_ = 2.0f;
    float close_speed_ = 3.0f;

    float detect_range_x_ = 5.f;
    float detect_range_y_ = 4.5f;

    float align_epsilon_ = 0.08f;
};

}  // namespace super_cave_boy
