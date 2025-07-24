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
                debug_draw.AddRect(Vector3f::Zero, shape.data.half, Vector4f(0, 0, 1, 0.5), &m);
            } break;
            default:
                break;
        }
    }

    debug_draw.Batch();
}

}  // namespace cave
