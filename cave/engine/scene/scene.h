#pragma once
#include "engine/assets/asset_interface.h"
#include "engine/core/base/noncopyable.h"
#include "engine/ecs/component_manager.h"
#include "engine/ecs/view.h"
#include "engine/math/ray.h"

// components
#include "engine/scene/scene_component.h"  // @TODO: split this

#include "engine/scene/animator_component.h"
#include "engine/scene/camera_component.h"
#include "engine/scene/collider_component.h"
#include "engine/scene/light_component.h"
#include "engine/scene/lua_script_component.h"
#include "engine/scene/material_component.h"
#include "engine/scene/mesh_renderer_component.h"
#include "engine/scene/sprite_renderer_component.h"
#include "engine/scene/tile_map_renderer_component.h"
#include "engine/scene/transform_component.h"

namespace cave::jobsystem {
class Context;
}

namespace cave {

#define REGISTER_COMPONENT_SERIALIZED_LIST                                           \
    REGISTER_COMPONENT(NameComponent, "World::NameComponent", 0)                     \
    REGISTER_COMPONENT(HierarchyComponent, "World::HierarchyComponent", 0)           \
    REGISTER_COMPONENT(TransformComponent, "World::TransformComponent", 0)           \
    REGISTER_COMPONENT(CameraComponent, "World::CameraComponent", 0)                 \
    REGISTER_COMPONENT(LightComponent, "World::LightComponent", 0)                   \
    REGISTER_COMPONENT(AnimatorComponent, "World::AnimatorComponent", 0)             \
    REGISTER_COMPONENT(ColliderComponent, "World::ColliderComponent", 0)             \
    REGISTER_COMPONENT(VelocityComponent, "World::VelocityComponent", 0)             \
    REGISTER_COMPONENT(LuaScriptComponent, "World::LuaScriptComponent", 0)           \
    REGISTER_COMPONENT(MeshRendererComponent, "World::MeshRendererComponent", 0)     \
    REGISTER_COMPONENT(MaterialComponent, "World::MaterialComponent", 0)             \
    REGISTER_COMPONENT(SpriteRendererComponent, "World::SpriteRendererComponent", 0) \
    REGISTER_COMPONENT(TileMapRendererComponent, "World::TileMapRendererComponent", 0)

// @TODO: use meta table for all components
#define REGISTER_COMPONENT_LIST                                            \
    REGISTER_COMPONENT_SERIALIZED_LIST                                     \
    REGISTER_COMPONENT(ArmatureComponent, "World::ArmatureComponent", 0)   \
    REGISTER_COMPONENT(AnimationComponent, "World::AnimationComponent", 0) \
    REGISTER_COMPONENT(RigidBodyComponent, "World::RigidBodyComponent", 0) \
    REGISTER_COMPONENT(ClothComponent, "World::ClothComponent", 0)         \
    REGISTER_COMPONENT(VoxelGiComponent, "World::VoxelGiComponent", 0)     \
    REGISTER_COMPONENT(EnvironmentComponent, "World::EnvironmentComponent", 0)

// @TODO: refactor
struct PhysicsWorldContext;

enum class PhysicsMode : uint8_t {
    NONE = 0,
    COLLISION_DETECTION,
    SIMULATION,
    COUNT,
};

enum SceneDirtyFlags : uint32_t {
    SCENE_DIRTY_NONE = BIT(0),
    SCENE_DIRTY_WORLD = BIT(1),
    SCENE_DIRTY_CAMERA = BIT(2),
    SCENE_DIRTY_LIGHT = BIT(3),
};
DEFINE_ENUM_BITWISE_OPERATIONS(SceneDirtyFlags);

class Scene : public NonCopyable, public IAsset {
    ecs::ComponentLibrary m_componentLib;

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
    inline T& GetComponentByIndex(size_t) { return *(T*)0; }
    template<ComponentType T>
    inline ecs::Entity GetEntityByIndex(size_t) { return ecs::Entity::Null(); }

    // @TODO: support View<A, B, ...>
    template<typename T>
    inline ecs::View<T> View() {
        static_assert(0, "this code should never instantiate");
        struct Dummy {};
        ecs::ComponentManager<Dummy> dummy;
        return ecs::View(dummy);
    }

    template<typename T>
    inline ecs::ConstView<T> View() const {
        static_assert(0, "this code should never instantiate");
        struct Dummy {};
        ecs::ComponentManager<Dummy> dummy;
        return ecs::ConstView(dummy);
    }

#pragma region WORLD_COMPONENTS_REGISTRY
#define REGISTER_COMPONENT(T, NAME, VER)                                                                           \
    ecs::ComponentManager<T>& m_##T##s = m_componentLib.RegisterManager<T>(NAME, VER);                             \
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
    T& Create<T>(const ecs::Entity& p_entity) { return m_##T##s.Create(p_entity); }                                \
    template<>                                                                                                     \
    inline ecs::View<T> View() { return ecs::View<T>(m_##T##s); }                                                  \
    template<>                                                                                                     \
    inline ecs::ConstView<T> View() const { return ecs::ConstView<T>(m_##T##s); }

#pragma endregion WORLD_COMPONENTS_REGISTRY

    REGISTER_COMPONENT_LIST
#undef REGISTER_COMPONENT

public:
    void Update(float p_delta_time);

    void Copy(Scene& p_other);

    void Merge(Scene& p_other);

    ecs::Entity GetMainCamera();

    ecs::Entity FindEntityByName(const char* p_name);

    void AttachChild(ecs::Entity p_entity, ecs::Entity p_parent);

    void AttachChild(ecs::Entity p_entity) { AttachChild(p_entity, m_root); }

    void RemoveEntity(ecs::Entity p_entity);

    auto LoadFromDisk(const AssetMetaData&) -> Result<void> override;

    auto SaveToDisk(const AssetMetaData&) const -> Result<void> override;

    virtual std::vector<Guid> GetDependencies() const override;

    struct RayIntersectionResult {
        ecs::Entity entity;
    };

    RayIntersectionResult Intersects(Ray& p_ray);
    bool RayObjectIntersect(ecs::Entity p_object_id, Ray& p_ray);

    const AABB& GetBound() const { return m_bound; }

    // @TODO: refactor
    ecs::Entity m_root;
    ecs::Entity m_selected;
    bool m_replace = false;

    std::atomic<uint32_t> m_dirtyFlags{ SCENE_DIRTY_NONE };
    // @TODO: refactor
    AABB m_bound;

    PhysicsMode m_physicsMode{ PhysicsMode::NONE };
    mutable PhysicsWorldContext* m_physicsWorld{ nullptr };

    const auto& GetLibraryEntries() const { return m_componentLib.m_entries; }
    SceneDirtyFlags GetDirtyFlags() const { return static_cast<SceneDirtyFlags>(m_dirtyFlags.load()); }

    ecs::Entity CreateEntity() { return ecs::Entity(++m_entity_seed); }

private:
    uint32_t m_entity_seed{ 0 };

    friend class EntityFactory;
};

}  // namespace cave
