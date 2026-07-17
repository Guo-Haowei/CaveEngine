#include "cave/runtime/ecs/components/HierarchyComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/ecs/components/SpriteRendererComponent.h"

#include "engine/private/render/renderer/FrameData.h"
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using namespace cave::math;

void RunSpriteRenderSystem(const Scene* scene, FrameData& framedata) {
    if (!scene) {
        return;
    }

    auto& sprites = framedata.sprites;

    auto view = scene->view<SpriteRendererComponent, TransformComponent, HierarchyComponent>();
    for (const auto& [id, sprite_renderer, transform, hier] : view) {
        if (!hier.visible()) continue;

        const Mat4f& world_matrix = transform.worldMatrix();
        PerBatchConstantBuffer batch_buffer;
        batch_buffer.c_worldMatrix = world_matrix;
        batch_buffer.c_tint_color = sprite_renderer.tintColor();
        const auto& rect = sprite_renderer.rect();
        batch_buffer.c_uv_rect = Vec4f(rect.min(), rect.max());

        DrawItem draw;
        draw.index.count = 6;
        draw.batch_idx = framedata.batchCache.FindOrAdd(id, batch_buffer);
        draw.z_index = sprite_renderer.zIndex();

        ImageAsset* image = sprite_renderer.handle().get();
        if (image) {
            draw.texture = image->gpu_texture.get();
        } else {
            // @TODO: dummy sprite?
        }

        sprites.push_back(draw);
    }

    std::sort(sprites.begin(), sprites.end(),
              [](const DrawItem& a, const DrawItem& b) {
                  return a.z_index < b.z_index;
              });
}

}  // namespace cave
