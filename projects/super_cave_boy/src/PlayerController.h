#pragma once
#include "cave/game/IGameModule.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

struct LegacyPlayerMotor {
    float speed = 4.0f;

    float hspeed = 0.0f;
    float vspeed = 0.0f;

    bool taking_jump = false;
    bool landed = false;
    bool grabbing = false;
    bool hurt = false;
    bool pausing = false;

    int health = 3;
    int sapphire = 0;

    // Direction face = Direction::Right;

    // PlayerState state = PlayerState::Jumping;
};

class PlayerController : public ::cave::NativeScript {
    using Entity = cave::ecs::Entity;

public:
    void onCreate() override;

    void onUpdate(float dt) override;

private:
    Entity animator_;

    LegacyPlayerMotor motor_;
};

}  // namespace super_cave_boy
