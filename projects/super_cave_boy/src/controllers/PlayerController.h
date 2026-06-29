#pragma once
#include "cave/core/math/Vector.h"
#include "cave/game/IGameModule.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/ecs/components/MovementComponent.h"
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

    bool isInvincible() const { return hurt_timer_ > 0.0f; }

    void takeDamage(const PlayerHurtInfo& info);
    void bounceFromEnemy(float bounce_speed);

    PlayerState state_ = PlayerState::Air;
    Entity animator_;

    float hurt_timer_ = 0.0f;
    float hurt_duration_ = 0.5f;

    bool hurt_ = false;
    bool taking_jump_ = false;
    bool landed_ = false;
    bool grabbing_ = false;

    float bounce_speed_ = 10.0f;

    float knockback_x_ = 8.0f;
    float knockback_y_ = 8.0f;

    // @TODO: clean up
    int health_ = 3;
    int sapphire_ = 0;
    float move_speed_ = 5.5f;
};

}  // namespace super_cave_boy
