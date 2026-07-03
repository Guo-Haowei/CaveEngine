#pragma once
#include "cave/core/base/NonCopyable.h"
#include "cave/core/math/AABB.h"
#include "cave/core/math/Ray.h"
#include "cave/runtime/assets/IAsset.h"
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
    void attachChild(ecs::Entity child) { attachChild(child, root_); }

    void update(float dt);
    void tick(SceneTickContext& ctx);

    void copy(const Scene& other);

    ecs::Entity duplicateEntity(ecs::Entity ent);

    void instantiatePrefab(PrefabInstanceComponent& prefab, ecs::Entity ent = ecs::Entity::Null());

    std::string_view name() const { return name_; }

    void onSimBegin(SceneContext& ctx);
    void onSimEnd(SceneContext& ctx);

    ecs::Entity findFirstByName(std::string_view name) const;
    ecs::Entity findChildByName(std::string_view name, ecs::Entity ent) const;

    ecs::ComponentStorage& storage() noexcept { return storage_; }
    const ecs::ComponentStorage& storage() const noexcept { return storage_; }
    SystemManager* systems() { return systems_.get(); }
    const SystemManager* systems() const { return systems_.get(); }
    const math::AABB& bound() const { return bound_; }
    void setBound(const math::AABB& bound) { bound_ = bound; }
    ecs::Entity root() const { return root_; }
    void setRoot(ecs::Entity root) { root_ = root; }

    auto loadFromDisk(const AssetMetaData&) -> Result<void> override;
    auto saveToDisk(const AssetMetaData&) const -> Result<void> override;
    virtual std::vector<Guid> dependencies() const override;

    // @TODO: deprecate
    SceneDirtyFlags dirtyFlags() const { return static_cast<SceneDirtyFlags>(dirtyFlags_.load()); }
    // @TODO: deprecate
    std::atomic<uint32_t> dirtyFlags_{ SCENE_DIRTY_NONE };

private:
    void simulate(SceneTickContext& ctx);

    std::vector<ecs::Entity> getSortedEntityArray() const;
    void flushPendingDestroy();

    ecs::ComponentRegistry& component_registry_;
    std::string name_;
    ecs::ComponentStorage storage_;

    uint32_t entity_seed_{ 0 };
    ecs::Entity root_;
    math::AABB bound_;

    std::unique_ptr<SystemManager> systems_;

    friend class AssimpImporter;
    friend class TinyGltfImporter;
};

}  // namespace cave
