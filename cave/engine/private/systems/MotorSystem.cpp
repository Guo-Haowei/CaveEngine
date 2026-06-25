#include "MotorSystem.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/MovementComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"

// @TODO: refactor
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using namespace cave::math;

MotorSystem::MotorSystem()
    : debug_id_(MakeDebugId(this)) {
}

void MotorSystem::update(float dt) {
    Scene& scene = context().scene;

    SceneQuery query(scene);
    const TileWorldSystem* tile_world = query.system<TileWorldSystem>();

    auto view = scene.view<MotorComponent, VelocityComponent, ColliderComponent, TransformComponent>();

    for (auto [ent, motor, vel, collider, trans] : view) {
        Vec2f delta = vel.linear.xy;
        delta *= dt;
        DEV_ASSERT(0 && tile_world && "TODO");
    }
}

}  // namespace cave
