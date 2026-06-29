#pragma once
#include "EnemyControllerBase.h"

namespace super_cave_boy {

class SnakeController final : public EnemyControllerBase {
public:
    void onCreate() override;
    void onUpdate(float dt) override;

private:
    int facing_x_;
};

}  // namespace super_cave_boy
