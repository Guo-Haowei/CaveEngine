#pragma once
#include "cave/core/base/NonCopyable.h"
#include "cave/core/math/AABB.h"
#include "cave/core/math/Ray.h"
#include "cave/runtime/ecs/ComponentStorage.h"
#include "cave/runtime/scene/SceneTickContext.h"

#include "engine/private/runtime/ecs/ComponentPool.h"
#include "engine/private/runtime/ecs/View.h"

// clang-format off
namespace cave::jobsystem { class Context; }
namespace cave::ecs { class ComponentRegistry; }
// clang-format on

namespace cave {

// @TODO: refactor
class PrefabInstanceComponent;
class SceneRuntime;
class SystemManager;

enum SceneDirtyFlags : uint32_t {
    SCENE_DIRTY_NONE = 0,
    SCENE_DIRTY_WORLD = BIT(0),
    SCENE_DIRTY_CAMERA = BIT(1),
    SCENE_DIRTY_LIGHT = BIT(2),
};
DEFINE_ENUM_BITWISE_OPERATIONS(SceneDirtyFlags);

class Scene final : public NonCopyable {
public:
    explicit Scene(ecs::ComponentRegistry& reg) noexcept;
    explicit Scene() noexcept;
    ~Scene();

    void* create(ComponentId cid, ecs::Entity ent) {
        return m_storage.createRaw(cid, ent);
    }

    template<ComponentType T>
    T& create(ecs::Entity ent) {
        return *((T*)m_storage.createRaw(T::kId, ent));
    }

    template<ComponentType T>
    T* component(ecs::Entity ent) {
        return (T*)m_storage.getRaw(T::kId, ent);
    }

    template<ComponentType T>
    const T* component(ecs::Entity ent) const {
        return (const T*)m_storage.getRaw(T::kId, ent);
    }

    bool has(ComponentId cid, ecs::Entity ent) const;
    size_t count(ComponentId cid) const;
    bool remove(ComponentId cid, ecs::Entity ent);

    template<ComponentType T>
    bool has(ecs::Entity ent) const { return has(T::kId, ent); }
    template<ComponentType T>
    size_t count() const { return count(T::kId); }
    template<ComponentType T>
    bool remove(ecs::Entity ent) { return remove(T::kId, ent); }

    // @TODO: remove depracated
    template<ComponentType T>
    [[deprecated]] T& getComponentByIndex(size_t idx) {
        if (auto* pool = (ecs::ComponentPool<T>*)m_storage.tryGet(T::kId)) {
            return pool->componentArray()[idx];
        }

        return *(T*)nullptr;
    }

    template<ComponentType T>
    [[deprecated]] ecs::Entity getEntityByIndex(size_t idx) {
        if (ecs::IComponentPool* pool = m_storage.tryGet(T::kId)) {
            return pool->entityArray()[idx];
        }

        return ecs::Entity::null();
    }

    template<ComponentType T>
    ecs::ComponentPool<T>* get() {
        return (ecs::ComponentPool<T>*)m_storage.tryGet(T::kId);
    }

    template<ComponentType T>
    const ecs::ComponentPool<T>* get() const {
        return (const ecs::ComponentPool<T>*)m_storage.tryGet(T::kId);
    }

    template<class... Cs>
    inline auto view() {
        return ecs::View<Cs...>(get<Cs>()...);
    }

    template<class... Cs>
    inline auto view() const {
        return ecs::ConstView<Cs...>(get<Cs>()...);
    }

    ecs::Entity createEntity() { return ecs::Entity(++m_entity_seed); }
    void removeEntity(ecs::Entity ent);

    void attachChild(ecs::Entity child, ecs::Entity parent);
    void attachChild(ecs::Entity child) { attachChild(child, m_root); }

    void begin(SceneTickContext ctx);
    void end();

    void tick(SceneTickContext ctx);
    void update(float dt);

    void copy(const Scene& other);

    ecs::Entity duplicateEntity(ecs::Entity ent);

    ecs::Entity findFirstByName(std::string_view name) const;
    ecs::Entity findChildByName(std::string_view name, ecs::Entity ent) const;

    auto& storage() noexcept { return m_storage; }
    const auto& storage() const noexcept { return m_storage; }

    auto* runtime() { return m_runtime.get(); }
    const auto* runtime() const { return m_runtime.get(); }

    const math::AABB& bound() const { return m_world_bound; }
    void setBound(const math::AABB& bound) { m_world_bound = bound; }

    ecs::Entity root() const { return m_root; }
    void setRoot(ecs::Entity root) { m_root = root; }

    void setSeed(uint32_t seed) { m_entity_seed = seed; }

    // @TODO: deprecate
    SceneDirtyFlags dirtyFlags() const { return static_cast<SceneDirtyFlags>(dirtyFlags_.load()); }
    // @TODO: deprecate
    std::atomic<uint32_t> dirtyFlags_{ SCENE_DIRTY_NONE };

    Vector<ecs::Entity> getSortedEntityArray() const;

private:
    void flushPendingDestroy();

    ecs::ComponentRegistry& m_component_registry;
    ecs::ComponentStorage m_storage;

    uint32_t m_entity_seed{ 0 };
    ecs::Entity m_root;
    math::AABB m_world_bound;

    Owner<SceneRuntime> m_runtime;
};

}  // namespace cave
