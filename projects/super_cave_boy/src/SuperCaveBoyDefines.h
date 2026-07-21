#pragma once
#include <cstdint>
#include "cave/core/math/Vec.h"
#include "cave/core/string/StringId.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"

namespace super_cave_boy {

constexpr float kPlayerKnockbackX = 7.f;
constexpr float kPlayerKnockbackY = 8.f;
constexpr float kPlayerMoveX = 5.9f;
constexpr float kPlayerHurtCountDown = 0.5f;
constexpr float kPlayerJumpForce = 13.0f;
constexpr float kPlayerWallJumpForce = 11.5f;
constexpr float kPlayerGrabEps = 0.03f;
constexpr float kExitAnimationDuration = 0.5f;

constexpr int kPlayerMaxHealth = 5;

constexpr uint32_t kPlayerLayer = 1;
constexpr uint32_t kEnemyLayer = 2;
constexpr uint32_t kLootLayer = 3;
constexpr uint32_t kExitLayer = 4;
constexpr uint32_t kCutsceneLayer = 5;
constexpr uint32_t kLavaLayer = 6;

constexpr cave::StringId kPlayerDamagedID = CAVE_SID("player.damaged");
constexpr cave::StringId kPlayerBouncedID = CAVE_SID("player.bounced");
constexpr cave::StringId kPlayerLeaveID = CAVE_SID("player.leave");
constexpr cave::StringId kCutsceneStartID = CAVE_SID("cutscene.start");
constexpr cave::StringId kCutsceneEndID = CAVE_SID("cutscene.end");
constexpr cave::StringId kGuardianAwakeID = CAVE_SID("guardian.awake");
constexpr cave::StringId kGuardianBeginFightID = CAVE_SID("guardian.begin_fight");
constexpr cave::StringId kGuardianDefeatedID = CAVE_SID("guardian.defeated");

constexpr cave::StringId kPlayerHealthID = CAVE_SID("player.health");
constexpr cave::StringId kLootCountID = CAVE_SID("loot.count");

inline bool IsPlayer(const cave::ColliderComponent& collider) {
    return collider.layer() == kPlayerLayer;
}

inline bool IsEnemy(const cave::ColliderComponent& collider) {
    return collider.layer() == kEnemyLayer;
}

inline bool IsLava(const cave::ColliderComponent& collider) {
    return collider.layer() == kLavaLayer;
}

}  // namespace super_cave_boy