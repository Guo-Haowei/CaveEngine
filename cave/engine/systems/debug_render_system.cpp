#pragma once
#include "engine/renderer/frame_data.h"
#include "engine/scene/scene.h"

namespace cave {

void RunDebugRenderSystem(const Scene* p_scene, FrameData& p_framedata) {
    if (!p_scene) {
        return;
    }

    DebugDraw& debug_draw = p_framedata.GetDebugDraw();

    auto view = p_scene->View<ColliderComponent, TransformComponent>();
    for (const auto& [id, collider, transform] : view) {
        if (!collider.GetDebugDraw()) continue;
        const Matrix4x4f& m = transform.GetWorldMatrix();
        const Shape& shape = collider.GetShape();
        switch (shape.type) {
            case ShapeType::Box: {
                Vector2f min = Vector2f::Zero - Vector2f(shape.data.half.xy);
                Vector2f max = Vector2f::Zero + Vector2f(shape.data.half.xy);
                debug_draw.AddBox2Frame(min, max, Vector4f(0, 0, 1, 0.9f), &m, 0.04f);
            } break;
            default:
                break;
        }
    }

    for (const auto& [id, tile_map_renderer, transform] : p_scene->View<TileMapRendererComponent, TransformComponent>()) {
        const auto& boxes = tile_map_renderer.GetBoxes();
        const Matrix4x4f& m = transform.GetWorldMatrix();
        for (const Box2& box : boxes) {
            debug_draw.AddBox2Frame(box.GetMin(), box.GetMax(), Vector4f(0, 1, 1, 0.9f), &m, 0.04f);
        }
    }

    debug_draw.Batch();
}

}  // namespace cave
