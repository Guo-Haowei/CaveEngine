// =============================================================================
// File: cave/runtime/scene/SceneQuery.h
// =============================================================================
#pragma once
#include "cave/core/math/Ray.h"
#include "cave/core/math/Vector.h"
#include "cave/runtime/ecs/Entity.h"

namespace cave {

class Scene;
using ComponentId = uint16_t;

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
    explicit SceneQuery(Scene& scene) noexcept
        : scene_(scene) {}

    ecs::Entity findFirstByName(std::string_view name) const;

    void* component(ComponentId cid, ecs::Entity ent);
    const void* component(ComponentId cid, ecs::Entity ent) const;

    size_t componentCount(ComponentId cid) const;

    RayHit raycast(math::Ray& ray, const RaycastFilter& filter) const;

private:
    Scene& scene_;
};

}  // namespace cave
