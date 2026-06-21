#pragma once
#include "cave/game/IGameModule.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/script/native/NativeScript.h"

// clang-format off
namespace cave { class SceneQuery; }
// clang-format on

namespace super_cave_boy {

enum class PlayerState {
    Idle,
    Walk,
    Air,
    Grab,
    Hurt,
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

    PlayerState state = PlayerState::Air;
};

class PlayerController : public ::cave::NativeScript {
    using Entity = cave::ecs::Entity;

public:
    void onCreate() override;

    void onUpdate(float dt) override;

private:
    void drawDebug();
    void updateAnimation(cave::SceneQuery& query);

    Entity animator_;

    LegacyPlayerMotor motor_;
};

}  // namespace super_cave_boy
