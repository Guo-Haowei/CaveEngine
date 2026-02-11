#include "engine/private/renderer/frame_data.h"
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SpriteRendererComponent.h"

namespace cave {

using namespace cave::math;

void RunSpriteRenderSystem(const Scene* p_scene, FrameData& p_framedata) {
    if (!p_scene) {
        return;
    }

    auto view = p_scene->View<SpriteRendererComponent, TransformComponent>();
    for (const auto& [id, sprite_renderer, transform] : view) {
        const Matrix4x4f& world_matrix = transform.GetWorldMatrix();
        PerBatchConstantBuffer batch_buffer;
        batch_buffer.c_worldMatrix = world_matrix;
        batch_buffer.c_tint_color = sprite_renderer.GetTintColor();
        const auto& rect = sprite_renderer.GetRect();
        batch_buffer.c_uv_rect = Vector4f(rect.Min(), rect.Max());

        DrawItem draw;
        draw.index.count = 6;
        draw.batch_idx = p_framedata.batchCache.FindOrAdd(id, batch_buffer);

        ImageAsset* image = sprite_renderer.GetHandle().Get();
        if (image) {
            draw.texture = image->gpu_texture.get();
        } else {
            // @TODO: dummy sprite?
        }

        p_framedata.sprites.push_back(draw);
    }
}

}  // namespace cave
