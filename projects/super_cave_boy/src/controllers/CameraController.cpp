#include "CameraController.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/scene/SceneContext.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;

void CameraController::onCreate(SceneContext& ctx) {
    target_ = ctx.query.findFirstByName("player");
}

void CameraController::onDestroy(SceneContext&) {
}

void CameraController::onUpdate(SceneContext& ctx, float dt) {
    followTarget(ctx, dt);
}

void CameraController::followTarget(cave::SceneContext& ctx, float dt) {
    if (!entity().IsValid() || !target_.IsValid()) {
        return;
    }

    float speed = 8.f * dt;
    speed = math::max(speed, 0.0f);
    SceneQuery& query = ctx.query;

    auto target_transform = query.component<TransformComponent>(target_);
    auto camera_transform = query.component<TransformComponent>(entity());

    const Vec3f target_pos = target_transform->translation();
    const Vec3f camera_pos = camera_transform->translation();

    Vec3f new_pos = camera_pos + (target_pos - camera_pos) * speed;
    new_pos.z = camera_pos.z;
    camera_transform->setTranslation(new_pos);

    // @TODO: clamp camera to edge
}

}  // namespace super_cave_boy
