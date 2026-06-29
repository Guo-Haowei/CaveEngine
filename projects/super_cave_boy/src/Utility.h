#pragma once
#include <cstdint>
#include "cave/core/math/Vector.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"

namespace super_cave_boy {

constexpr uint32_t kPlayerLayer = 1;
constexpr uint32_t kEnemyLayer = 2;

constexpr float kBounceSpeed = 10.f;
constexpr float kKnockbackX = 7.f;
constexpr float kKnockbackY = 8.f;
constexpr float kPlayerMoveX = 5.5f;
constexpr float kHurtCountDown = 0.5f;

inline bool IsPlayer(const cave::ColliderComponent& collider) {
    return collider.layer() & kPlayerLayer;
}

inline bool IsEnemy(const cave::ColliderComponent& collider) {
    return collider.layer() & kEnemyLayer;
}

// @TODO: move to core/time/
class CountdownTimer {
public:
    CountdownTimer() = default;

    explicit CountdownTimer(float duration)
        : duration_(duration)
        , remaining_(0.0f) {}

    void start() {
        remaining_ = duration_;
    }

    void start(float duration) {
        duration_ = duration;
        remaining_ = duration;
    }

    void stop() {
        remaining_ = 0.0f;
    }

    void tick(float dt) {
        remaining_ = cave::math::max(0.0f, remaining_ - dt);
    }

    bool active() const {
        return remaining_ > 0.0f;
    }

    bool finished() const {
        return remaining_ <= 0.0f;
    }

    float remaining() const {
        return remaining_;
    }

    float duration() const {
        return duration_;
    }

private:
    float duration_ = 0.0f;
    float remaining_ = 0.0f;
};

}  // namespace super_cave_boy