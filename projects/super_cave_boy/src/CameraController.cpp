#include "CameraController.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/scene/SceneContext.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;

void CameraController::onCreate() {
    const SceneQuery query(context().scene);

    target_ = query.findFirstByName("player");
}

void CameraController::onDestroy() {
}

void CameraController::onUpdate(float dt) {
    followTarget(dt);
}

void CameraController::followTarget(float dt) {
    if (!entity().IsValid() || !target_.IsValid()) {
        return;
    }

    float speed = 8.f * dt;
    speed = math::max(speed, 0.0f);
    SceneQuery query(context().scene);

    auto target_transform = static_cast<const TransformComponent*>(query.component(TransformComponent_Id, target_));
    auto camera_transform = static_cast<TransformComponent*>(query.component(TransformComponent_Id, entity()));

    const Vec3f target_pos = target_transform->GetTranslation();
    const Vec3f camera_pos = camera_transform->GetTranslation();

    Vec3f new_pos = camera_pos + (target_pos - camera_pos) * speed;
    new_pos.z = camera_pos.z;
    camera_transform->SetTranslation(new_pos);

    // @TODO: clamp camera to edge
}

}  // namespace super_cave_boy
