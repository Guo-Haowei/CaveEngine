#pragma once
#include "cave/core/math/Vector.h"
#include "cave/game/IGameModule.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/ecs/components/MovementComponent.h"
#include "cave/runtime/script/native/NativeScript.h"

#include "Utility.h"

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

struct PlayerHurtInfo {
    int damage{ 1 };
    cave::math::Vec2f knockback{};
};

class PlayerController final : public ::cave::NativeScript {
    using Entity = cave::ecs::Entity;

protected:
    void onCreate() override;
    void onUpdate(float dt) override;
    void onCollision(cave::ecs::Entity other) override;

private:
    void updateAnimation(cave::SceneQuery& query);
    void updatePlayerState(cave::VelocityComponent& vel);

    void tryJump(cave::VelocityComponent& vel,
                 cave::MotorComponent& motor);

    void takeDamage(const PlayerHurtInfo& info);
    void bounceFromEnemy(float bounce_speed);
    bool hurt() const { return hurt_timer_.active(); }

    PlayerState state_ = PlayerState::Air;
    CountdownTimer hurt_timer_{ kHurtCountDown };

    Entity animator_;

    // @TODO: clean up
    bool taking_jump_ = false;
    bool landed_ = false;
    bool grabbing_ = false;

    int health_ = 3;
    int sapphire_ = 0;
};

}  // namespace super_cave_boy
