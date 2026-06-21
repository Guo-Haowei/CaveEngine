#include "SnakeController.h"

#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/ecs/components/VelocityComponent.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace super_cave_boy {

using namespace ::cave;

void SnakeController::onCreate() {
}

void SnakeController::onUpdate(float dt) {
    SceneQuery query(context().scene);

    auto transform = query.component<TransformComponent>(entity());
    auto vel = query.component<VelocityComponent>(entity());
    DEV_ASSERT(transform && vel);

    if (std::fmod(elapsed_, 6.f) < 3.f) {
        vel->linear.x = 1.0f;
    } else {
        vel->linear.x = -1.0f;
    }

    elapsed_ += dt;

    const float dx = vel->linear.x * dt * 2.0f;
    transform->Translate({ dx, 0.0f, 0.0f });
}

}  // namespace super_cave_boy
