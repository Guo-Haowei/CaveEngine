#include "BatController.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/runtime/ecs/components/VelocityComponent.h"
#include "cave/runtime/platformer/FacingComponent.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"

#include "platformer/PlatformerCollision.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;

namespace {

}  // namespace

void BatController::onCreate() {
}

void BatController::onUpdate(float dt) {
    SceneQuery query(context().scene);

    auto transform = query.component<TransformComponent>(entity());
    auto collider = query.component<ColliderComponent>(entity());
    auto vel = query.component<VelocityComponent>(entity());

    DEV_ASSERT(transform && collider && vel);

    unused(dt);
}

}  // namespace super_cave_boy

