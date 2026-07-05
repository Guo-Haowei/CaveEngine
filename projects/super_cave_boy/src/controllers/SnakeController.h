#pragma once
#include "EnemyControllerBase.h"

namespace super_cave_boy {

class SnakeController final : public EnemyControllerBase {
public:
    void onCreate(cave::SceneContext& ctx) override;
    void onUpdate(cave::SceneContext& ctx, float dt) override;

private:
    int m_facing_x;
};

}  // namespace super_cave_boy
