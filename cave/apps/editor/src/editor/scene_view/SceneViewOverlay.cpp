#include "SceneViewOverlay.h"

#include "cave/runtime/display/ICanvas.h"
#include "cave/runtime/ecs/components/CameraComponent.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/PrefabInstanceComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using namespace ::cave::math;

static Box2 CameraOrthoAABB2(const CameraComponent& camera) {
    const float h = camera.orthoHeight();
    const float w = h * camera.aspect();

    const Vec3f& p = camera.position();

    return Box2{
        Vec2f(p.x - w * 0.5f, p.y - h * 0.5f),
        Vec2f(p.x + w * 0.5f, p.y + h * 0.5f),
    };
}

void SceneViewOverlay::drawSelectionHighlight(ICanvas& canvas,
                                              ViewId view_id,
                                              const Scene& scene,
                                              ecs::Entity ent) {
    drawSelectionHighlightImpl(canvas, view_id, scene, ent);
}

void SceneViewOverlay::drawSelectionHighlightImpl(ICanvas& canvas,
                                                  ViewId view_id,
                                                  const Scene& scene,
                                                  ecs::Entity ent) {
    if (ent.isNull()) return;

    const auto transform = scene.component<TransformComponent>(ent);
    if (!transform) return;

    canvas.pushView(view_id);

    constexpr Vec4f kColliderColor{ 0.0f, 0.0f, 1.0f, 0.9f };
    constexpr Vec4f kCameraColor{ 0.6f, 0.8f, 0.7f, 0.9f };

    const Mat4f& m = transform->worldMatrix();
    if (const auto collider = scene.component<ColliderComponent>(ent)) {
        const Shape& shape = collider->shape();

        switch (shape.type) {
            case ShapeType::Box: {
                Vec2f min = Vec2f::Zero - Vec2f(shape.data.half.xy);
                Vec2f max = Vec2f::Zero + Vec2f(shape.data.half.xy);
                Draw2DOptions options{
                    .z_index = INT_MAX,
                    .tint = kColliderColor,
                    .transform = &m,
                };
                canvas.addBox2Frame(min, max, 0.04f, options);
            } break;
            default:
                break;
        }
    }

    if (const auto camera = scene.component<CameraComponent>(ent)) {
        if (camera->isOrtho()) {
            Box2 box = CameraOrthoAABB2(*camera);
            Draw2DOptions options{
                .z_index = INT_MAX,
                .tint = kCameraColor,
                .transform = &m,
            };
            canvas.addBox2Frame(box.min(), box.max(), 0.2f, options);
        }
    }

    canvas.popView();
}

}  // namespace cave
