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
struct PhysicsWorldContext;
class PrefabInstanceComponent;

enum class PhysicsMode : uint8_t {
    NONE = 0,
    COLLISION_DETECTION,
    SIMULATION,
    COUNT,
};

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

    explicit Scene(std::string p_name, ecs::ComponentRegistry& p_reg) noexcept;
    explicit Scene(std::string p_name) noexcept;
    ~Scene() = default;

    template<ComponentType T>
    T& Create(ecs::Entity p_ent) {
        return *((T*)m_storage.CreateRaw(p_ent, T::kId));
    }

    template<ComponentType T>
    T* GetComponent(ecs::Entity p_ent) {
        return (T*)m_storage.GetRaw(p_ent, T::kId);
    }

    template<ComponentType T>
    const T* GetComponent(ecs::Entity p_ent) const {
        return (const T*)m_storage.GetRaw(p_ent, T::kId);
    }

    bool Has(ComponentId p_cid, ecs::Entity p_ent) const;
    size_t GetCount(ComponentId p_cid) const;
    bool Remove(ComponentId p_cid, ecs::Entity p_ent);

    template<ComponentType T>
    bool Has(ecs::Entity p_ent) const { return Has(T::kId, p_ent); }
    template<ComponentType T>
    size_t GetCount() const { return GetCount(T::kId); }
    template<ComponentType T>
    bool Remove(ecs::Entity p_ent) { return Remove(T::kId, p_ent); }

    // @TODO: remove depracated
    template<ComponentType T>
    [[deprecated]] T& GetComponentByIndex(size_t p_idx) {
        if (auto* pool = (ecs::ComponentPool<T>*)m_storage.TryGet(T::kId)) {
            return pool->GetComponentArray()[p_idx];
        }

        return *(T*)nullptr;
    }

    template<ComponentType T>
    [[deprecated]] ecs::Entity GetEntityByIndex(size_t p_idx) {
        if (ecs::IComponentPool* pool = m_storage.TryGet(T::kId)) {
            return pool->GetEntityArray()[p_idx];
        }

        return ecs::Entity::Null();
    }

    template<ComponentType T>
    ecs::ComponentPool<T>* Get() {
        return (ecs::ComponentPool<T>*)m_storage.TryGet(T::kId);
    }

    template<ComponentType T>
    const ecs::ComponentPool<T>* Get() const {
        return (const ecs::ComponentPool<T>*)m_storage.TryGet(T::kId);
    }

    template<class... Cs>
    inline auto View() {
        return ecs::View<Cs...>(Get<Cs>()...);
    }

    template<class... Cs>
    inline auto View() const {
        return ecs::ConstView<Cs...>(Get<Cs>()...);
    }

    ecs::Entity CreateEntity() { return ecs::Entity(++m_entity_seed); }
    void RemoveEntity(ecs::Entity p_ent);

    void AttachChild(ecs::Entity p_child, ecs::Entity p_parent);
    void AttachChild(ecs::Entity p_child) { AttachChild(p_child, m_root); }

    void Update(float p_delta_time);

    void Copy(const Scene& p_other);

    ecs::Entity DuplicateEntity(ecs::Entity p_ent);

    void InstantiatePrefab(PrefabInstanceComponent& p_prefab, ecs::Entity p_ent = ecs::Entity::Null());

    const math::AABB& GetBound() const { return m_bound; }

    ecs::Entity m_root;

    // @TODO: deprecate
    std::atomic<uint32_t> m_dirtyFlags{ SCENE_DIRTY_NONE };
    // @TODO: refactor
    math::AABB m_bound;

    // @TODO: refactor
    PhysicsMode m_physicsMode{ PhysicsMode::NONE };

    // @TODO: refactor
    mutable PhysicsWorldContext* m_physicsWorld{ nullptr };

    // @TODO: refactor
    SceneDirtyFlags GetDirtyFlags() const { return static_cast<SceneDirtyFlags>(m_dirtyFlags.load()); }

    // -------------------------------------------------------------------------
    // Utility
    // -------------------------------------------------------------------------
    ecs::Entity FindEntityByName(std::string_view p_name) const;

    // -------------------------------------------------------------------------
    // IAsset
    // -------------------------------------------------------------------------
    auto LoadFromDisk(const AssetMetaData&) -> Result<void> override;

    auto SaveToDisk(const AssetMetaData&) const -> Result<void> override;

    virtual std::vector<Guid> GetDependencies() const override;

    // -------------------------------------------------------------------------
    // Accessor
    // -------------------------------------------------------------------------
    ecs::ComponentStorage& Storage() noexcept { return m_storage; }
    const ecs::ComponentStorage& Storage() const noexcept { return m_storage; }

    std::string_view Name() const { return m_name; }

private:
    std::vector<ecs::Entity> GetSortedEntityArray() const;

    ecs::ComponentRegistry& m_reg;
    std::string m_name;
    ecs::ComponentStorage m_storage;

    uint32_t m_entity_seed{ 0 };

    friend class AssimpImporter;
    friend class TinyGltfImporter;
    friend class SceneQuery;
};

}  // namespace cave
