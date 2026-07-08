// =============================================================================
// File: cave/runtime/scene/SceneQuery.h
// =============================================================================
#pragma once
#include <string>
#include <string_view>

#include "cave/core/math/Ray.h"
#include "cave/core/math/Vec.h"
#include "cave/core/ids/Entity.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

class ISceneSystem;
class Scene;
enum class SceneSystemId : uint32_t;

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
        : m_scene(scene) {}

    ISceneSystem* system(SceneSystemId id);
    template<typename T>
    T* system() {
        return static_cast<T*>(system(T::kSystemId));
    }

    ecs::Entity findFirstByName(std::string_view name) const;
    ecs::Entity findChildByName(std::string_view name, ecs::Entity ent) const;

    void queueDestroy(ecs::Entity ent);

    void* addComponent(ComponentId cid, ecs::Entity ent);
    template<ComponentType T>
    T* addComponent(ecs::Entity ent) { return (T*)addComponent(T::kId, ent); }

    void* component(ComponentId cid, ecs::Entity ent);
    const void* component(ComponentId cid, ecs::Entity ent) const;
    template<ComponentType T>
    T* component(ecs::Entity ent) { return (T*)component(T::kId, ent); }
    template<ComponentType T>
    const T* component(ecs::Entity ent) const { return (const T*)component(T::kId, ent); }

    size_t componentCount(ComponentId cid) const;

    RayHit raycast(math::Ray& ray, const RaycastFilter& filter) const;

    std::string debugString() const;

private:
    Scene& m_scene;
};

}  // namespace cave
