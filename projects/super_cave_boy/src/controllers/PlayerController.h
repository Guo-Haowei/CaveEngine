#pragma once
#include "cave/game/IGameModule.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/script/native/NativeScript.h"

// clang-format off
namespace cave { class SceneQuery; }
namespace cave { struct VelocityComponent; }
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

    bool taking_jump = false;
    bool landed = false;
    bool grabbing = false;
    bool hurt = false;
    bool pausing = false;

    PlayerState state = PlayerState::Air;
};

class PlayerController final : public ::cave::NativeScript {
    using Entity = cave::ecs::Entity;

public:

private:
    void onCreate() override;
    void onUpdate(float dt) override;
    void onCollision(cave::ecs::Entity other) override;

    void updateAnimation(cave::SceneQuery& query);
    void updatePlayerState(cave::VelocityComponent& vel);

    Entity animator_;

    LegacyPlayerMotor motor_;
};

}  // namespace super_cave_boy
