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
    void start(cave::SceneContext& ctx) override;
    void update(cave::SceneContext& ctx, float dt) override;

private:
    void updateAnimation(cave::SpriteAnimatorComponent& animator);
    void updatePlayerState(cave::VelocityComponent& vel);

    void tryJump(cave::VelocityComponent& vel,
                 cave::MotorComponent& motor);
    bool hurt() const { return m_hurt_timer.active(); }

    PlayerState m_state = PlayerState::Air;
    CountdownTimer m_hurt_timer{ kPlayerHurtCountDown };

    Entity m_animator;

    // @TODO: clean up
    bool m_taking_jump_ = false;
    bool m_landed_ = false;
    bool m_grabbing_ = false;
};

}  // namespace super_cave_boy
