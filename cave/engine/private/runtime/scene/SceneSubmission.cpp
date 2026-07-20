#include "SceneSubmission.h"

#include "Scene.h"

#include "cave/render/components/BackgroundComponent.h"
#include "cave/render/components/SpriteRendererComponent.h"
#include "cave/runtime/display/ICanvas.h"
#include "cave/runtime/ecs/components/HierarchyComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/tile_map/TileMapInstanceComponent.h"
#include "cave/runtime/tile_map/TileSetAsset.h"

// private
#include "engine/private/core/math/geomath.h"
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/view/ResolvedView.h"

namespace cave {

using namespace ::cave::math;

namespace {

// @TODO: SpriteAnimation as well
const TileFrame* FindTileFrame(const TileDefinition& definition,
                               float elapsed_time) {
    if (definition.animation.empty()) {
        return nullptr;
    }

    float total_duration = 0.0f;
    for (const TileFrame& frame : definition.animation) {
        total_duration += std::max(frame.duration, 0.0f);
    }

    if (total_duration <= 0.0f) {
        return &definition.animation.front();
    }

    float local_time = std::fmod(elapsed_time, total_duration);
    if (local_time < 0.0f) {
        local_time += total_duration;
    }

    for (const TileFrame& frame : definition.animation) {
        const float duration = std::max(frame.duration, 0.0f);

        if (local_time < duration) {
            return &frame;
        }

        local_time -= duration;
    }

    // Handles floating-point precision near total_duration.
    return &definition.animation.back();
}

// @TODO: move to animation system
void SubmitTileLayer(const SceneSubmitContext& ctx,
                     const TileMapInstanceComponent::LayerCache& layer,
                     const TransformComponent& transform) {
    const ImageAsset* image = layer.image.get();
    const TileSetAsset* tile_set = layer.tile_set.get();
    if (!image || !tile_set) {
        return;
    }

    const auto& frames = tile_set->frames();
    for (const auto& tile : layer.tiles) {
        // @TODO: move to somewhere else
        tile.elapsed += ctx.dt;
        const auto* definition = tile_set->getTileDefinition(tile.tile_id);
        if (!definition) continue;

        uint32_t atlas_index = definition->id;
        if (!definition->animation.empty()) {
            auto* frame = FindTileFrame(*definition, tile.elapsed);
            if (!frame) continue;
            atlas_index = frame->atlas_index;
        }

        if (atlas_index >= frames.size()) continue;
        const auto frame = frames[atlas_index];

        ImageDrawOptions options;
        options.z_index = layer.z_index;
        options.transform = &transform.worldMatrix();
        options.uv_min = frame.min();
        options.uv_max = frame.max();

        const float s = 1.0f;
        float x0 = s * tile.x;
        float y0 = s * tile.y;
        float x1 = s * (tile.x + 1);
        float y1 = s * (tile.y + 1);

        ctx.canvas.addImage(image->gpu_texture.get(),
                            Vec2f(x0, y0),
                            Vec2f(x1, y1),
                            options);
    }
}

void SubmitTileMaps(const SceneSubmitContext& ctx, Scene& scene) {
    auto view = scene.view<TileMapInstanceComponent, TransformComponent, HierarchyComponent>();

    for (const auto& [id, instance, transform, hier] : view) {
        if (!hier.visible()) continue;

        instance.createRenderData();

        instance.tileMapHandle();

        auto layers = instance.layers();
        if (layers.empty()) continue;

        for (const auto& layer : layers) {
            SubmitTileLayer(ctx, layer, transform);
        }
    }
}

void SubmitSprites(const SceneSubmitContext& ctx, Scene& scene) {
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

        ImageDrawOptions options;

        options.tint = renderer.tintColor();
        options.z_index = renderer.zIndex();

        if (const auto* background = scene.component<BackgroundComponent>(id)) {
            const Vec2f uv_min = view_min * background->parallax / background->repeat_size;
            const Vec2f uv_max = view_max * background->parallax / background->repeat_size;

            options.transform = &world;
            options.uv_min = uv_min;
            options.uv_max = uv_max;
            continue;
        } else {
            const auto& rect = renderer.rect();

            options.transform = &transform.worldMatrix();
            options.uv_min = rect.min();
            options.uv_max = rect.max();
        }

        ctx.canvas.addImage(image->gpu_texture.get(),
                            Vec2f(-0.5f, -0.5f),
                            Vec2f(0.5f, 0.5f),
                            options);
    }
}

}  // namespace

void SubmitScene(const ResolvedView& view,
                 const SceneSubmitContext& ctx) {

    if (!view.scene) {
        return;
    }

    ctx.canvas.pushView(view.view_id);
    SubmitTileMaps(ctx, *view.scene);
    SubmitSprites(ctx, *view.scene);
    ctx.canvas.popView();

    //std::sort(framedata.sprites.begin(), framedata.sprites.end(),
    //          [](const DrawItem& a, const DrawItem& b) {
    //              return a.z_index < b.z_index;
    //          });
}

}  // namespace cave
