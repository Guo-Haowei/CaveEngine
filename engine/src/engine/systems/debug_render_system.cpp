#pragma once
#include "engine/renderer/frame_data.h"
#include "engine/scene/scene.h"

namespace cave {

void RunDebugRenderSystem(const Scene* p_scene, FrameData& p_framedata) {
    DebugDraw& debug_draw = p_framedata.GetDebugDraw();

    for (const auto& [id, collider] : p_scene->View<ColliderComponent>()) {
        const TransformComponent* transform = p_scene->GetComponent<TransformComponent>(id);
        if (!transform) continue;
        const Matrix4x4f& m = transform->GetWorldMatrix();
        const Shape& shape = collider.GetShape();
        switch (shape.type) {
            case ShapeType::Box: {
                Vector2f min = Vector2f::Zero - Vector2f(shape.data.half.xy);
                Vector2f max = Vector2f::Zero + Vector2f(shape.data.half.xy);
                debug_draw.AddBox2Frame(min, max, Vector4f(0, 0, 1, 0.9f), &m);
            } break;
            default:
                break;
        }
    }

    debug_draw.Batch();
}

}  // namespace cave
