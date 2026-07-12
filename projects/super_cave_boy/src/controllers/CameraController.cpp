#include "CameraController.h"

#include "cave/runtime/ecs/components/CameraComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;

namespace {

Vec2f ClampCameraToTileMap(Vec2f camera_pos,
                           Vec2f map_min,
                           Vec2f map_max,
                           float ortho_height,
                           float aspect_ratio) {
    const float half_h = ortho_height * 0.5f;
    const float half_w = ortho_height * aspect_ratio * 0.5f;

    const Vec2f half_view{ half_w, half_h };

    Vec2f min_pos = map_min + half_view;
    Vec2f max_pos = map_max - half_view;

    if (min_pos.x > max_pos.x) {
        camera_pos.x = (map_min.x + map_max.x) * 0.5f;
    } else {
        camera_pos.x = std::clamp(camera_pos.x, min_pos.x, max_pos.x);
    }

    if (min_pos.y > max_pos.y) {
        camera_pos.y = (map_min.y + map_max.y) * 0.5f;
    } else {
        camera_pos.y = std::clamp(camera_pos.y, min_pos.y, max_pos.y);
    }

    return camera_pos;
}

}  // namespace

void CameraController::start() {
    m_target = query().findFirstByName("player");
}

void CameraController::update(float dt) {
    followTarget(dt);
}

void CameraController::followTarget(float dt) {
    if (!entity().valid() || !m_target.valid()) {
        return;
    }

    float speed = 8.f * dt;
    speed = math::max(speed, 0.0f);

    auto target_transform = query().component<TransformComponent>(m_target);
    auto camera_transform = component<TransformComponent>();
    auto camera = component<CameraComponent>();

    const Vec3f target_pos = target_transform->translation();
    const Vec3f camera_pos = camera_transform->translation();

    Vec3f new_pos = camera_pos + (target_pos - camera_pos) * speed;

    auto tile_world = system<TileWorldSystem>();
    auto bound = tile_world->worldBound();

    Vec2f xy = ClampCameraToTileMap(new_pos.xy, bound.min(), bound.max(), camera->orthoHeight(), camera->aspect());

    camera_transform->setTranslation(Vec3f{ xy, camera_pos.z });
}

}  // namespace super_cave_boy
