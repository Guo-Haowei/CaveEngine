#include "engine/assets/image_asset.h"
#include "engine/renderer/frame_data.h"
#include "engine/scene/scene.h"
#include "engine/scene/sprite_renderer_component.h"

namespace cave {

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
        batch_buffer.c_uv_rect = Vector4f(rect.GetMin(), rect.GetMax());

        DrawCommand draw;
        draw.index_count = 6;
        draw.batch_idx = p_framedata.batchCache.FindOrAdd(id, batch_buffer);

        ImageAsset* image = sprite_renderer.GetHandle().Get();
        if (image) {
            draw.texture = image->gpu_texture.get();
        } else {
            // @TODO: dummy sprite?
        }

        p_framedata.sprites.push_back(RenderCommand::From(draw));
    }
}

}  // namespace cave
