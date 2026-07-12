#pragma once
#include <cstdint>
#include "cave/core/math/Vec.h"
#include "cave/core/string/StringId.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"

namespace super_cave_boy {

using namespace ::cave::literals;

constexpr uint32_t kPlayerLayer = 1;
constexpr uint32_t kEnemyLayer = 2;

constexpr float kPlayerBounceSpeed = 10.f;
constexpr float kPlayerKnockbackX = 7.f;
constexpr float kPlayerKnockbackY = 8.f;
constexpr float kPlayerMoveX = 5.7f;
constexpr float kPlayerHurtCountDown = 0.5f;
constexpr float kPlayerStompTolerance = 0.12f;
constexpr float kPlayerJumpForce = 13.0f;
constexpr float kPlayerWallJumpForce = 11.5f;
constexpr float kPlayerGrabEps = 0.03f;
constexpr float kExitAnimationDuration = 0.5f;

constexpr cave::StringId kPlayerDamaged = "player.damaged"_sid;
constexpr cave::StringId kPlayerBounced = "player.bounced"_sid;
constexpr cave::StringId kPlayerLeave = "player.leave"_sid;
constexpr cave::StringId kCutsceneStart = "cutscene.start"_sid;
constexpr cave::StringId kCutsceneEnd = "cutscene.end"_sid;
constexpr cave::StringId kGuardianAwake = "guardian.awake"_sid;
constexpr cave::StringId kGuardianBeginFight = "guardian.begin_fight"_sid;
constexpr cave::StringId kGuardianDefeated = "guardian.defeated"_sid;

inline bool IsPlayer(const cave::ColliderComponent& collider) {
    return collider.layer() & kPlayerLayer;
}

inline bool IsEnemy(const cave::ColliderComponent& collider) {
    return collider.layer() & kEnemyLayer;
}

}  // namespace super_cave_boy