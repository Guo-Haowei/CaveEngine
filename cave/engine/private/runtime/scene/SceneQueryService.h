#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/SceneId.h"
#include "cave/core/math/Vector.h"
#include "cave/core/math/Ray.h"

namespace cave {

class SceneRegistry;

struct RayHit {
    bool hit{ false };
    ecs::Entity entity{};
    float t{ 0.0f };
    math::Vector3f position{ 0 };
    math::Vector3f normal{ 0 };
    uint32_t submesh = 0;
    uint32_t triangle = 0;
};

struct RaycastFilter {
    uint32_t layer_mask = 0xFFFFFFFFu;
    bool backface_cull = true;
    bool closest_only = true;
};

class SceneQueryService {
public:
    explicit SceneQueryService(SceneRegistry& p_scene_reg) noexcept
        : m_scene_reg(p_scene_reg) {
    }

    RayHit Raycast(SceneId p_scene_id,
                   math::Ray& p_ray,
                   const RaycastFilter& p_filter);

private:
    SceneRegistry& m_scene_reg;
};

}  // namespace cave
