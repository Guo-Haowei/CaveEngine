#pragma once
#include "cave/core/base/NonCopyable.h"
#include "cave/core/math/AABB.h"
#include "cave/core/math/Ray.h"
#include "cave/runtime/assets/IAsset.h"
#include "cave/runtime/ecs/ComponentStorage.h"

#include "engine/private/runtime/ecs/ComponentPool.h"
#include "engine/private/runtime/ecs/View.h"

// clang-format off
namespace cave::jobsystem { class Context; }
namespace cave::ecs { class ComponentRegistry; }
// clang-format on

namespace cave {

// @TODO: refactor
class PrefabInstanceComponent;

struct SceneContext;
class SystemManager;

enum SceneDirtyFlags : uint32_t {
    SCENE_DIRTY_NONE = 0,
    SCENE_DIRTY_WORLD = BIT(0),
    SCENE_DIRTY_CAMERA = BIT(1),
    SCENE_DIRTY_LIGHT = BIT(2),
};
DEFINE_ENUM_BITWISE_OPERATIONS(SceneDirtyFlags);

class Scene final : public NonCopyable, public IAsset {
    CAVE_ASSET(Scene, AssetType::Scene, 0)

public:
    static constexpr const char* EXTENSION = ".scene";

    explicit Scene(std::string name, ecs::ComponentRegistry& reg) noexcept;
    explicit Scene(std::string name) noexcept;
    ~Scene() override;

    void* create(ComponentId cid, ecs::Entity ent) {
        return storage_.CreateRaw(ent, cid);
    }

    template<ComponentType T>
    T& create(ecs::Entity ent) {
        return *((T*)storage_.CreateRaw(ent, T::kId));
    }

    template<ComponentType T>
    T* component(ecs::Entity ent) {
        return (T*)storage_.GetRaw(ent, T::kId);
    }

    template<ComponentType T>
    const T* component(ecs::Entity ent) const {
        return (const T*)storage_.GetRaw(ent, T::kId);
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
        if (auto* pool = (ecs::ComponentPool<T>*)storage_.TryGet(T::kId)) {
            return pool->GetComponentArray()[idx];
        }

        return *(T*)nullptr;
    }

    template<ComponentType T>
    [[deprecated]] ecs::Entity getEntityByIndex(size_t idx) {
        if (ecs::IComponentPool* pool = storage_.TryGet(T::kId)) {
            return pool->GetEntityArray()[idx];
        }

        return ecs::Entity::Null();
    }

    template<ComponentType T>
    ecs::ComponentPool<T>* get() {
        return (ecs::ComponentPool<T>*)storage_.TryGet(T::kId);
    }

    template<ComponentType T>
    const ecs::ComponentPool<T>* get() const {
        return (const ecs::ComponentPool<T>*)storage_.TryGet(T::kId);
    }

    template<class... Cs>
    inline auto view() {
        return ecs::View<Cs...>(get<Cs>()...);
    }

    template<class... Cs>
    inline auto view() const {
        return ecs::ConstView<Cs...>(get<Cs>()...);
    }

    ecs::Entity createEntity() { return ecs::Entity(++entity_seed_); }
    void removeEntity(ecs::Entity ent);

    void attachChild(ecs::Entity child, ecs::Entity parent);
    void attachChild(ecs::Entity child) { attachChild(child, m_root); }

    void update(float dt);

    void copy(const Scene& other);

    ecs::Entity duplicateEntity(ecs::Entity ent);

    void instantiatePrefab(PrefabInstanceComponent& prefab, ecs::Entity ent = ecs::Entity::Null());

    const math::AABB& bound() const { return m_bound; }

    ecs::Entity m_root;

    // @TODO: deprecate
    std::atomic<uint32_t> m_dirtyFlags{ SCENE_DIRTY_NONE };
    // @TODO: refactor
    math::AABB m_bound;

    // @TODO: refactor
    SceneDirtyFlags dirtyFlags() const { return static_cast<SceneDirtyFlags>(m_dirtyFlags.load()); }

    ecs::ComponentStorage& storage() noexcept { return storage_; }
    const ecs::ComponentStorage& storage() const noexcept { return storage_; }

    std::string_view name() const { return name_; }

    void onSimBegin(SceneContext& ctx);
    void onSimEnd();
    void simulate(float dt);

    // -------------------------------------------------------------------------
    // Utility
    // -------------------------------------------------------------------------
    ecs::Entity findFirstByName(std::string_view name) const;
    ecs::Entity findChildByName(std::string_view name, ecs::Entity ent) const;

    // -------------------------------------------------------------------------
    // IAsset
    // -------------------------------------------------------------------------
    auto LoadFromDisk(const AssetMetaData&) -> Result<void> override;

    auto SaveToDisk(const AssetMetaData&) const -> Result<void> override;

    virtual std::vector<Guid> GetDependencies() const override;

private:
    std::vector<ecs::Entity> GetSortedEntityArray() const;

    ecs::ComponentRegistry& component_registry_;
    std::string name_;
    ecs::ComponentStorage storage_;

    uint32_t entity_seed_{ 0 };

    std::unique_ptr<SystemManager> systems_;

    friend class AssimpImporter;
    friend class TinyGltfImporter;
    friend class SceneQuery;
};

}  // namespace cave
