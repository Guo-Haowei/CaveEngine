#include "engine/assets/image_asset.h"
#include "engine/renderer/frame_data.h"
#include "engine/scene/scene.h"

namespace cave {

void RunTileMapRenderSystem(Scene* p_scene, FrameData& p_framedata) {
    if (!p_scene) {
        return;
    }

    Scene& scene = *p_scene;
    auto view = scene.View<TileMapRendererComponent>();
    for (const auto& [id, tile_map_renderer] : view) {
        tile_map_renderer.UpdateData();

        const auto& cache = tile_map_renderer.GetCache();
        if (!tile_map_renderer.GetVisibility()) {
            continue;
        }

        if (!cache.mesh) {
            continue;
        }

        const TransformComponent& transform = *scene.GetComponent<TransformComponent>(id);

        const Matrix4x4f& world_matrix = transform.GetWorldMatrix();
        PerBatchConstantBuffer batch_buffer;
        batch_buffer.c_worldMatrix = world_matrix;
        batch_buffer.c_tint_color = tile_map_renderer.GetTintColor();

        DrawCommand draw;
        draw.index_count = cache.mesh->desc.drawCount;
        draw.mesh_data = cache.mesh.get();
        draw.batch_idx = p_framedata.batchCache.FindOrAdd(id, batch_buffer);

        ImageAsset* image = cache.image.Get();
        if (image) {
            draw.texture = image->gpu_texture.get();
        } else {
            // @TODO: dummy sprite?
        }

        p_framedata.tile_maps.push_back(RenderCommand::From(draw));
    }
}

}  // namespace cave
