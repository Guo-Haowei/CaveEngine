#include "cave/render/components/BackgroundRendererComponent.h"
#include "cave/render/components/SpriteRendererComponent.h"
#include "cave/runtime/ecs/components/HierarchyComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/tile_map/TileMapInstanceComponent.h"

#include "engine/private/render/renderer/FrameData.h"
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using namespace cave::math;

namespace {

void CollectTileMap(Scene& scene, FrameData& framedata) {
    auto view = scene.view<TileMapInstanceComponent, HierarchyComponent>();

    for (const auto& [id, instance, hier] : view) {
        if (!hier.visible()) continue;

        instance.createRenderData();

        const auto& cache = instance.cache();
        if (!instance.visible()) continue;

        if (!cache.mesh) continue;

        const TransformComponent& transform = *scene.component<TransformComponent>(id);

        const math::Mat4f& world_matrix = transform.worldMatrix();
        PerBatchConstantBuffer batch_buffer;
        batch_buffer.c_worldMatrix = world_matrix;
        batch_buffer.c_tint_color = instance.tintColor();

        DrawItem draw;
        draw.index.count = cache.mesh->desc.drawCount;
        draw.mesh_data = cache.mesh.get();
        draw.batch_idx = framedata.batchCache.FindOrAdd(id, batch_buffer);

        ImageAsset* image = cache.image.get();
        draw.texture = image ? image->gpu_texture.get() : nullptr;

        framedata.sprites.push_back(draw);
    }
}

void CollectSprites(const Scene& scene, FrameData& framedata) {
    auto& sprites = framedata.sprites;
    if (scene.count<BackgroundRendererComponent>()) {
        auto camera_id = scene.activeCamera();
        DEV_ASSERT(camera_id.valid());
        const auto* camera = scene.component<CameraComponent>(camera_id);
        const auto* camera_transform = scene.component<TransformComponent>(camera_id);
        DEV_ASSERT(camera && camera_transform);
        DEV_ASSERT(camera->isOrtho());

        const float half_height = camera->orthoHeight();
        const float half_width = half_height * camera->aspect();

        const Vec2f camera_pos = camera_transform->translation().xy;

        const Vec2f view_size = Vec2f{ half_width * 2.0f, half_height * 2.0f };
        const Vec2f view_min = camera_pos - Vec2f{ half_width, half_height };
        const Vec2f view_max = view_min + view_size;

        for (const auto& [id, background, hier] :
             scene.view<BackgroundRendererComponent, HierarchyComponent>()) {
            if (!hier.visible()) continue;

            const ImageAsset* image = background.handle().get();
            if (!image) {
                continue;
            }

            const Mat4f world =
                glm::translate(glm::vec3(camera_pos.x, camera_pos.y, 0.0f)) *
                glm::scale(glm::vec3(view_size.x, view_size.y, 1.0f));

            const Vec2f uv_min = view_min * background.parallax() / background.repeatSize();
            const Vec2f uv_max = view_max * background.parallax() / background.repeatSize();

            PerBatchConstantBuffer batch;
            batch.c_worldMatrix = world;
            batch.c_tint_color = background.tint();
            batch.c_uv_rect = Vec4f(uv_min, uv_max);

            DrawItem draw;
            draw.index.count = 6;
            draw.batch_idx = framedata.batchCache.FindOrAdd(id, batch);
            draw.texture = image->gpu_texture.get();
            // @TODO: fix this
            draw.mesh_data = nullptr;
            draw.z_index = -10;

            sprites.push_back(draw);
        }
    }

    for (const auto& [id, renderer, transform, hier] :
         scene.view<SpriteRendererComponent, TransformComponent, HierarchyComponent>()) {
        if (!hier.visible()) continue;

        const Mat4f& world_matrix = transform.worldMatrix();
        PerBatchConstantBuffer batch_buffer;
        batch_buffer.c_worldMatrix = world_matrix;
        batch_buffer.c_tint_color = renderer.tintColor();
        const auto& rect = renderer.rect();
        batch_buffer.c_uv_rect = Vec4f(rect.min(), rect.max());

        DrawItem draw;
        draw.index.count = 6;
        draw.batch_idx = framedata.batchCache.FindOrAdd(id, batch_buffer);
        draw.z_index = renderer.zIndex();

        ImageAsset* image = renderer.handle().get();
        draw.texture = image->gpu_texture.get();

        sprites.push_back(draw);
    }
}

}  // namespace

void RunSpriteRenderSystem(Scene* scene, FrameData& framedata) {
    if (!scene) {
        return;
    }

    CollectTileMap(*scene, framedata);
    CollectSprites(*scene, framedata);

    std::sort(framedata.sprites.begin(), framedata.sprites.end(),
              [](const DrawItem& a, const DrawItem& b) {
                  return a.z_index < b.z_index;
              });
}

}  // namespace cave
