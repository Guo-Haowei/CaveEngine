#include "cave/runtime/ecs/components/SpriteRendererComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"

#include "engine/private/render/renderer/FrameData.h"
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using namespace cave::math;

void RunSpriteRenderSystem(const Scene* p_scene, FrameData& p_framedata) {
    if (!p_scene) {
        return;
    }

    auto view = p_scene->view<SpriteRendererComponent, TransformComponent>();
    for (const auto& [id, sprite_renderer, transform] : view) {
        const Mat4f& world_matrix = transform.worldMatrix();
        PerBatchConstantBuffer batch_buffer;
        batch_buffer.c_worldMatrix = world_matrix;
        batch_buffer.c_tint_color = sprite_renderer.tintColor();
        const auto& rect = sprite_renderer.rect();
        batch_buffer.c_uv_rect = Vec4f(rect.min(), rect.max());

        DrawItem draw;
        draw.index.count = 6;
        draw.batch_idx = p_framedata.batchCache.FindOrAdd(id, batch_buffer);

        ImageAsset* image = sprite_renderer.handle().get();
        if (image) {
            draw.texture = image->gpu_texture.get();
        } else {
            // @TODO: dummy sprite?
        }

        p_framedata.sprites.push_back(draw);
    }
}

}  // namespace cave
