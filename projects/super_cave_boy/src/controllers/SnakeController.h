#pragma once
#include "EnemyControllerBase.h"

namespace super_cave_boy {

class SnakeController final : public EnemyControllerBase {
public:
    void start() override;
    void update(float dt) override;

private:
    int m_facing_x;
};

}  // namespace super_cave_boy
