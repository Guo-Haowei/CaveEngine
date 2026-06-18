#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/render/renderer/FrameData.h"

#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/ecs/components/TileMapRendererComponent.h"

namespace cave {

void RunTileMapRenderSystem(Scene* p_scene, FrameData& p_framedata) {
    if (!p_scene) {
        return;
    }

    Scene& scene = *p_scene;
    auto view = scene.view<TileMapRendererComponent>();
    for (const auto& [id, tile_map_renderer] : view) {
        tile_map_renderer.CreateRenderData();

        const auto& cache = tile_map_renderer.GetCache();
        if (!tile_map_renderer.IsVisible()) {
            continue;
        }

        if (!cache.mesh) {
            continue;
        }

        const TransformComponent& transform = *scene.component<TransformComponent>(id);

        const math::Mat4f& world_matrix = transform.GetWorldMatrix();
        PerBatchConstantBuffer batch_buffer;
        batch_buffer.c_worldMatrix = world_matrix;
        batch_buffer.c_tint_color = tile_map_renderer.GetTintColor();

        DrawItem draw;
        draw.index.count = cache.mesh->desc.drawCount;
        draw.mesh_data = cache.mesh.get();
        draw.batch_idx = p_framedata.batchCache.FindOrAdd(id, batch_buffer);

        ImageAsset* image = cache.image.Get();
        if (image) {
            draw.texture = image->gpu_texture.get();
        } else {
            // @TODO: dummy sprite?
        }

        p_framedata.tile_maps.push_back(draw);
    }
}

}  // namespace cave
