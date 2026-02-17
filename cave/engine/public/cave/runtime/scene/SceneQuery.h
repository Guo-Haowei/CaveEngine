// =============================================================================
// File: public/cave/runtime/scene/SceneQuery.h
// =============================================================================
#pragma once
#include "cave/core/math/Ray.h"
#include "cave/core/math/Vector.h"
#include "cave/runtime/ecs/Entity.h"

namespace cave {

class Scene;

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

class SceneQuery {
public:
    explicit SceneQuery(const Scene& p_scene) noexcept
        : m_scene(p_scene) {}

    ecs::Entity FindFirstEntity(std::string_view p_name);

    RayHit Raycast(math::Ray& p_ray, const RaycastFilter& p_filter);

private:
    const Scene& m_scene;
};

}  // namespace cave
