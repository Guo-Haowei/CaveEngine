#pragma once
#include "engine/private/render/renderer/FrameData.h"
#include "engine/private/runtime/scene/Scene.h"

#include "engine/private/runtime/ecs/components/All.h"

namespace cave {

using namespace cave::math;

void RunDebugRenderSystem(const Scene* scene, FrameData& framedata) {
    if (!scene) {
        return;
    }

    DebugDrawService& debug_draw = framedata.GetDebugDraw();

    auto view = scene->view<ColliderComponent, TransformComponent>();
    for (const auto& [id, collider, transform] : view) {
        if (!collider.GetDebugDraw()) continue;
        const Mat4f& m = transform.GetWorldMatrix();
        const Shape& shape = collider.GetShape();
        switch (shape.type) {
            case ShapeType::Box: {
                Vec2f min = Vec2f::Zero - Vec2f(shape.data.half.xy);
                Vec2f max = Vec2f::Zero + Vec2f(shape.data.half.xy);
                debug_draw.addBox2Frame(min, max, Vec4f(0, 0, 1, 0.9f), &m, 0.04f);
            } break;
            default:
                break;
        }
    }

    debug_draw.batch();
}

}  // namespace cave
