#pragma once
#include "cave/core/NonCopyable.h"
#include "cave/core/math/Ray.h"
#include "cave/runtime/assets/IAsset.h"
#include "cave/runtime/ecs/CameraComponent.h"
#include "cave/runtime/ecs/ComponentRegistry.h"
#include "cave/runtime/ecs/HierarchyComponent.h"
#include "cave/runtime/ecs/LuaScriptComponent.h"
#include "cave/runtime/ecs/NameComponent.h"
#include "cave/runtime/ecs/TransformComponent.h"

#include "engine/private/runtime/ecs/ComponentManager.h"
#include "engine/private/runtime/ecs/View.h"

// components
// @TODO: split this
#include "engine/private/runtime/scene/SceneComponent.h"

#include "engine/private/runtime/scene/ColliderComponent.h"
#include "engine/private/runtime/scene/LightComponent.h"
#include "engine/private/runtime/scene/MaterialComponent.h"
#include "engine/private/runtime/scene/MeshRendererComponent.h"
#include "engine/private/runtime/scene/SkeletalAnimationComponent.h"
#include "engine/private/runtime/scene/SpriteAnimatorComponent.h"
#include "engine/private/runtime/scene/SpriteRendererComponent.h"
#include "engine/private/runtime/scene/TileMapRendererComponent.h"

namespace cave::jobsystem {
class Context;
}

namespace cave {

// Tags that don't need to be serialized
struct NoSaveTag {};

#define REGISTER_COMPONENT(TYPE, ...) \
    template<>                        \
    struct IsComponent<TYPE> : std::true_type {};
REGISTER_COMPONENT_LIST
#undef REGISTER_COMPONENT

// @TODO: refactor
struct PhysicsWorldContext;

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

class Scene : public NonCopyable, public IAsset {
    ecs::ComponentLibrary m_component_lib;

    CAVE_ASSET(Scene, AssetType::Scene, 0)
public:
    static constexpr const char* EXTENSION = ".scene";

public:
    template<ComponentType T>
    const T* GetComponent(const ecs::Entity&) const { return nullptr; }
    template<ComponentType T>
    T* GetComponent(const ecs::Entity&) { return nullptr; }
    template<ComponentType T>
    bool Contains(const ecs::Entity&) const { return false; }
    template<ComponentType T>
    size_t GetCount() const { return 0; }
    template<ComponentType T>
    T& Create(const ecs::Entity&) { return *(T*)(nullptr); }
    template<ComponentType T>
    void Remove(const ecs::Entity&) {}

    template<ComponentType T>
    inline T& GetComponentByIndex(size_t) { return *(T*)0; }
    template<ComponentType T>
    inline ecs::Entity GetEntityByIndex(size_t) { return ecs::Entity::Null(); }

    template<ComponentType T>
    inline const ecs::ComponentManager<T>& Get() const {
        static_assert(0, "this code should never instantiate");
        return *((ecs::ComponentManager<T>*)nullptr);
    }

    template<ComponentType T>
    inline ecs::ComponentManager<T>& Get() {
        static_assert(0, "this code should never instantiate");
        return *((ecs::ComponentManager<T>*)nullptr);
    }

    template<class... Cs>
    inline auto View() {
        return ecs::View<Cs...>(Get<Cs>()...);
    }

    template<class... Cs>
    inline auto View() const {
        return ecs::ConstView<Cs...>(Get<Cs>()...);
    }

#pragma region WORLD_COMPONENTS_REGISTRY
#define REGISTER_COMPONENT(T, NAME, VER)                                                                           \
    ecs::ComponentManager<T>& m_##T##s = m_component_lib.RegisterManager<T>(NAME, VER);                            \
    template<>                                                                                                     \
    inline T& GetComponentByIndex<T>(size_t p_index) { return m_##T##s.m_componentArray[p_index]; }                \
    template<>                                                                                                     \
    inline ecs::Entity GetEntityByIndex<T>(size_t p_index) { return m_##T##s.m_entityArray[p_index]; }             \
    template<>                                                                                                     \
    inline const T* GetComponent<T>(const ecs::Entity& p_entity) const { return m_##T##s.GetComponent(p_entity); } \
    template<>                                                                                                     \
    inline T* GetComponent<T>(const ecs::Entity& p_entity) { return m_##T##s.GetComponent(p_entity); }             \
    template<>                                                                                                     \
    inline bool Contains<T>(const ecs::Entity& p_entity) const { return m_##T##s.Contains(p_entity); }             \
    template<>                                                                                                     \
    inline size_t GetCount<T>() const { return m_##T##s.GetCount(); }                                              \
    template<>                                                                                                     \
    inline T& Create<T>(const ecs::Entity& p_entity) { return m_##T##s.Create(p_entity); }                         \
    template<>                                                                                                     \
    inline void Remove<T>(const ecs::Entity& p_entity) { return m_##T##s.Remove(p_entity); }                       \
    template<>                                                                                                     \
    inline const ecs::ComponentManager<T>& Get() const { return m_##T##s; }                                        \
    template<>                                                                                                     \
    inline ecs::ComponentManager<T>& Get() { return m_##T##s; }

#pragma endregion WORLD_COMPONENTS_REGISTRY

    REGISTER_COMPONENT_LIST
#undef REGISTER_COMPONENT

public:
    void Update(float p_delta_time);

    void Copy(const Scene& p_other);

    ecs::Entity FindEntityByName(const char* p_name);

    ecs::Entity DuplicateEntity(ecs::Entity p_entity);

    void InstantiatePrefab(PrefabInstanceComponent& p_prefab, ecs::Entity p_entity = ecs::Entity::Null());

    auto LoadFromDisk(const AssetMetaData&) -> Result<void> override;

    auto SaveToDisk(const AssetMetaData&) const -> Result<void> override;

    virtual std::vector<Guid> GetDependencies() const override;

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
    const auto& GetLibraryEntries() const { return m_component_lib.m_entries; }
    SceneDirtyFlags GetDirtyFlags() const { return static_cast<SceneDirtyFlags>(m_dirtyFlags.load()); }

private:
    ecs::Entity CreateEntity() { return ecs::Entity(++m_entity_seed); }

    void AttachChild(ecs::Entity p_child, ecs::Entity p_parent);
    void AttachChild(ecs::Entity p_child) { AttachChild(p_child, m_root); }

    std::vector<ecs::Entity> GetSortedEntityArray() const;

    uint32_t m_entity_seed{ 0 };

    friend class SceneEdit;
    friend class AssimpImporter;
    friend class TinyGltfImporter;
};

}  // namespace cave
