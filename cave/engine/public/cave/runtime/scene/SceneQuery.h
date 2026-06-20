// =============================================================================
// File: cave/runtime/scene/SceneQuery.h
// =============================================================================
#pragma once
#include <string_view>
#include "cave/core/math/Ray.h"
#include "cave/core/math/Vector.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/ecs/ComponentDefines.h"
#include "cave/runtime/scene/ISceneSystem.h"

namespace cave {

class Scene;
using ComponentId = uint16_t;

struct RayHit {
    bool hit{ false };
    ecs::Entity entity{};
    float t{ 0.0f };
    math::Vec3f position{ 0 };
    math::Vec3f normal{ 0 };
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

    ISceneSystem* system(SceneSystemId id);
    template<SceneSystem T>
    T* system() {
        return static_cast<T*>(system(T::kSystemId));
    }

    ecs::Entity findFirstByName(std::string_view name) const;

    void* component(ComponentId cid, ecs::Entity ent);
    const void* component(ComponentId cid, ecs::Entity ent) const;
    template<ComponentType T>
    T* component(ecs::Entity ent) { return (T*)component(T::kId, ent); }
    template<ComponentType T>
    const T* component(ecs::Entity ent) const { return (const T*)component(T::kId, ent); }

    size_t componentCount(ComponentId cid) const;

    RayHit raycast(math::Ray& ray, const RaycastFilter& filter) const;

private:
    Scene& scene_;
};

}  // namespace cave
