#pragma once
#include "cave/core/math/Vector.h"
#include "cave/game/IGameModule.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/ecs/components/MovementComponent.h"
#include "cave/runtime/ecs/components/SpriteAnimatorComponent.h"
#include "cave/runtime/script/native/NativeScript.h"

#include "Utility.h"

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

public:
    void takeDamage(cave::VelocityComponent& vel,
                    cave::MotorComponent& motor,
                    const PlayerHurtInfo& info);

    void bounceFromEnemy(cave::VelocityComponent& vel,
                         cave::MotorComponent& motor,
                         float bounce_speed);

protected:
    void onCreate(cave::SceneContext& ctx) override;
    void onUpdate(cave::SceneContext& ctx, float dt) override;

private:
    void updateAnimation(cave::SpriteAnimatorComponent& animator);
    void updatePlayerState(cave::VelocityComponent& vel);

    void tryJump(cave::VelocityComponent& vel,
                 cave::MotorComponent& motor);
    bool hurt() const { return hurt_timer_.active(); }

    PlayerState state_ = PlayerState::Air;
    CountdownTimer hurt_timer_{ kPlayerHurtCountDown };

    Entity animator_;

    // @TODO: clean up
    bool taking_jump_ = false;
    bool landed_ = false;
    bool grabbing_ = false;

    int health_ = 3;
    int sapphire_ = 0;
};

}  // namespace super_cave_boy
