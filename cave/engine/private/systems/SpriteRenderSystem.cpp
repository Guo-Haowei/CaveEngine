#include "cave/render/components/BackgroundComponent.h"
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

        auto layers = instance.layers();
        if (layers.empty()) continue;

        const TransformComponent& transform = *scene.component<TransformComponent>(id);

        const Mat4f& world_matrix = transform.worldMatrix();
        PerBatchConstantBuffer batch;
        batch.c_worldMatrix = world_matrix;
        batch.c_tint_color = Vec4f::One;

        for (const auto& layer : layers) {
            if (!layer.isStatic()) {
                // CRASH_NOW();
            }
            if (!layer.visible) continue;
            if (!layer.mesh) continue;
            if (layer.mesh->desc.drawCount == 0) continue;
            ImageAsset* image = layer.image.get();
            if (!image) continue;

            DrawItem draw;
            draw.index.count = layer.mesh->desc.drawCount;
            draw.mesh_data = layer.mesh.get();
            draw.batch_idx = framedata.batchCache.FindOrAdd(id, batch);
            draw.z_index = layer.z_index;

            draw.texture = image->gpu_texture.get();
            framedata.sprites.push_back(draw);
        }
    }
}

void CollectSprites(const Scene& scene, FrameData& framedata) {
    auto& sprites = framedata.sprites;

    Vec2f view_min;
    Vec2f view_max;
    Mat4f world{};
    auto camera_id = scene.activeCamera();
    if (scene.count<BackgroundComponent>() && camera_id.valid()) {
        const auto* camera = scene.component<CameraComponent>(camera_id);
        const auto* camera_transform = scene.component<TransformComponent>(camera_id);
        DEV_ASSERT(camera && camera_transform);
        DEV_ASSERT(camera->isOrtho());

        const float half_height = camera->orthoHeight();
        const float half_width = half_height * camera->aspect();

        const Vec2f camera_pos = camera_transform->translation().xy;

        const Vec2f view_size = Vec2f{ half_width * 2.0f, half_height * 2.0f };
        view_min = camera_pos - Vec2f{ half_width, half_height };
        view_max = view_min + view_size;

        world = glm::translate(glm::vec3(camera_pos.x, camera_pos.y, 0.0f)) *
                glm::scale(glm::vec3(view_size.x, view_size.y, 1.0f));
    }

    auto view = scene.view<SpriteRendererComponent, TransformComponent, HierarchyComponent>();
    for (const auto& [id, renderer, transform, hier] : view) {
        if (!hier.visible()) continue;
        ImageAsset* image = renderer.handle().get();
        if (!image) continue;

        PerBatchConstantBuffer batch;
        batch.c_tint_color = renderer.tintColor();

        if (const auto* background = scene.component<BackgroundComponent>(id)) {
            const Vec2f uv_min = view_min * background->parallax / background->repeat_size;
            const Vec2f uv_max = view_max * background->parallax / background->repeat_size;
            batch.c_worldMatrix = world;
            batch.c_uv_rect = Vec4f(uv_min, uv_max);
        } else {
            const auto& rect = renderer.rect();
            batch.c_worldMatrix = transform.worldMatrix();
            batch.c_uv_rect = Vec4f(rect.min(), rect.max());
        }

        DrawItem draw;
        draw.index.count = 6;
        draw.batch_idx = framedata.batchCache.FindOrAdd(id, batch);
        draw.z_index = renderer.zIndex();
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
