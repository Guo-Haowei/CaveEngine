#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/render/renderer/FrameData.h"

#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/tile_map/TileMapInstanceComponent.h"

namespace cave {

void RunTileMapRenderSystem(Scene* scene, FrameData& framedata) {
    if (!scene) {
        return;
    }

    auto view = scene->view<TileMapInstanceComponent>();
    for (const auto& [id, tile_map_renderer] : view) {
        tile_map_renderer.createRenderData();

        const auto& cache = tile_map_renderer.cache();
        if (!tile_map_renderer.visible()) {
            continue;
        }

        if (!cache.mesh) {
            continue;
        }

        const TransformComponent& transform = *scene->component<TransformComponent>(id);

        const math::Mat4f& world_matrix = transform.worldMatrix();
        PerBatchConstantBuffer batch_buffer;
        batch_buffer.c_worldMatrix = world_matrix;
        batch_buffer.c_tint_color = tile_map_renderer.tintColor();

        DrawItem draw;
        draw.index.count = cache.mesh->desc.drawCount;
        draw.mesh_data = cache.mesh.get();
        draw.batch_idx = framedata.batchCache.FindOrAdd(id, batch_buffer);

        ImageAsset* image = cache.image.get();
        if (image) {
            draw.texture = image->gpu_texture.get();
        } else {
            // @TODO: dummy sprite?
        }

        framedata.tile_maps.push_back(draw);
    }
}

}  // namespace cave
