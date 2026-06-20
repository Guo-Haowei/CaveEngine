#pragma once
#include "cave/game/IGameModule.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

enum class PlayerState {
    Idle,
    Walk,
    Air,
    Grab,
    Hurt,
};

enum class Facing {
    Left,
    Right,
};

struct LegacyPlayerMotor {
    const float speed = 5.5f;

    float hspeed = 0.0f;
    float vspeed = 0.0f;

    bool taking_jump = false;
    bool landed = false;
    bool grabbing = false;
    bool hurt = false;
    bool pausing = false;

    Facing face = Facing::Right;
    PlayerState state = PlayerState::Air;
};

class PlayerController : public ::cave::NativeScript {
    using Entity = cave::ecs::Entity;

public:
    void onCreate() override;

    void onUpdate(float dt) override;

private:
    void drawDebug();

    Entity animator_;

    LegacyPlayerMotor motor_;
};

}  // namespace super_cave_boy
