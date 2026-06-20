#pragma once
#include "cave/game/IGameModule.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

struct LegacyPlayerMotor {
    float hspeed = 0.0f;  // -1, 0, 1
    float vspeed = 0.0f;
    float speed = 4.0f;

    bool grabbing = false;
    bool taking_jump = false;

    // Direction face = Direction::Right;
};

class PlayerController : public ::cave::NativeScript {
    using Entity = cave::ecs::Entity;

public:
    void onCreate() override;
    void onDestroy() override;

    void onUpdate(float dt) override;

private:
    Entity animator_;

    LegacyPlayerMotor motor_;
};

}  // namespace super_cave_boy
