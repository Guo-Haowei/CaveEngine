#pragma once
#include "EnemyControllerBase.h"

namespace super_cave_boy {

class SnakeController final : public EnemyControllerBase {
public:
    void start(cave::SceneContext& ctx) override;
    void update(cave::SceneContext& ctx, float dt) override;

private:
    int m_facing_x;
};

}  // namespace super_cave_boy
