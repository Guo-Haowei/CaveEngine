#include "SceneViewOverlay.h"

#include "cave/render/ICanvas.h"
#include "cave/runtime/ecs/components/CameraComponent.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using namespace ::cave::math;

void SceneViewOverlay::drawSelectionHighlight(ICanvas& canvas,
                                              ViewId view_id,
                                              const Scene& scene,
                                              ecs::Entity ent) {
    if (ent.isNull()) return;

    const auto transform = scene.component<TransformComponent>(ent);
    if (!transform) return;

    canvas.pushView(view_id);

    const Mat4f& m = transform->worldMatrix();
    if (const auto collider = scene.component<ColliderComponent>(ent)) {
        const Shape& shape = collider->shape();

        switch (shape.type) {
            case ShapeType::Box: {
                Vec2f min = Vec2f::Zero - Vec2f(shape.data.half.xy);
                Vec2f max = Vec2f::Zero + Vec2f(shape.data.half.xy);
                canvas.addBox2Frame(min, max, 0.04f, Vec4f(0, 0, 1, 0.9f), &m);
            } break;
            default:
                break;
        }
    }

    if (const auto camera = scene.component<CameraComponent>(ent)) {
    }

    canvas.popView();
}

}  // namespace cave
