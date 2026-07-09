#pragma once
#include "cave/render/ICanvas.h"

#include "engine/private/render/renderer/FrameData.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/ecs/components/All.h"

namespace cave {

using namespace cave::math;

void RunDebugRenderSystem(const Scene* scene, ICanvas& debug_draw) {
    if (!scene) {
        return;
    }

    auto view = scene->view<ColliderComponent, TransformComponent>();
    for (const auto& [id, collider, transform] : view) {
        if (!collider.debugDraw()) continue;
        const Mat4f& m = transform.worldMatrix();
        const Shape& shape = collider.shape();
        switch (shape.type) {
            case ShapeType::Box: {
                Vec2f min = Vec2f::Zero - Vec2f(shape.data.half.xy);
                Vec2f max = Vec2f::Zero + Vec2f(shape.data.half.xy);
                debug_draw.addBox2Frame(min, max, Vec4f(0, 0, 1, 0.9f), 0.04f, &m);
            } break;
            default:
                break;
        }
    }
}

}  // namespace cave