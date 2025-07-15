#include "engine/renderer/frame_data.h"
#include "engine/assets/assets.h"
#include "engine/scene/scene.h"

namespace my {

void RunTileMapRenderSystem(Scene* p_scene, FrameData& p_framedata) {
    if (!p_scene) {
        return;
    }
    Scene& scene = *p_scene;
    auto view = scene.View<TileMapRenderer>();
    for (const auto [id, tile_map] : view) {
        tile_map.CreateRenderData();

        for (const auto& layer : tile_map.GetLayerCache()) {
            if (!layer.mesh) {
                continue;
            }

            const TransformComponent& transform = *scene.GetComponent<TransformComponent>(id);

            const Matrix4x4f& world_matrix = transform.GetWorldMatrix();
            PerBatchConstantBuffer batch_buffer;
            batch_buffer.c_worldMatrix = world_matrix;

            DrawCommand draw;
            draw.indexCount = layer.mesh->desc.drawCount;
            draw.mesh_data = layer.mesh.get();
            draw.batch_idx = p_framedata.batchCache.FindOrAdd(id, batch_buffer);

            ImageAsset* image = layer.image.Get();
            if (image) {
                draw.texture = image->gpu_texture.get();
            }

            p_framedata.tile_maps.push_back(RenderCommand::From(draw));
        }
    }
}

}  // namespace my
