#include "Scene.h"

#include "cave/core/diagnostics/Profiler.h"
#include "engine/private/core/io/archive.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/systems/animation_system.h"
#include "engine/private/systems/ecs_systems.h"
#include "engine/private/systems/job_system/job_system.h"

// @TODO: refactor
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/serialization/yaml_include.h"

namespace cave::ecs {

// instantiate ComponentManagers
#define REGISTER_COMPONENT(TYPE, ...) template class ComponentPool<::cave::TYPE>;
REGISTER_COMPONENT_LIST
#undef REGISTER_COMPONENT

}  // namespace cave::ecs

namespace cave {

using ecs::Entity;
using namespace cave::math;

void Scene::Update(float p_timestep) {
    CAVE_PROFILE_EVENT();

    m_dirtyFlags.store(0);

    jobsystem::Context ctx;
    // animation
    RunSpriteAnimationSystem(*this, ctx, p_timestep);
    RunLightUpdateSystem(*this, ctx, p_timestep);
    RunAnimationUpdateSystem(*this, ctx, p_timestep);
    ctx.Wait();
    // transform, update local matrix from position, rotation and scale
    RunTransformationUpdateSystem(*this, ctx, p_timestep);
    ctx.Wait();
    // hierarchy, update world matrix based on hierarchy
    RunHierarchyUpdateSystem(*this, ctx, p_timestep);
    ctx.Wait();

    // mesh particles
    // RunMeshEmitterUpdateSystem(*this, ctx, p_timestep);
    // particle
    // RunParticleEmitterUpdateSystem(*this, ctx, p_timestep);

    RunSkeletonUpdateSystem(*this, ctx, p_timestep);
    ctx.Wait();

    // update bounding box
    RunMeshAABBUpdateSystem(*this, ctx, p_timestep);

    // @TODO: refactor
    for (auto [entity, camera, transform] : View<CameraComponent, TransformComponent>()) {
        if (camera.Update(transform.GetWorldMatrix())) {
            m_dirtyFlags.fetch_or(SCENE_DIRTY_CAMERA);
        }
    }

// @TODO: refactor
#if 0
    if (DVAR_GET_BOOL(gfx_bvh_generate)) {
        CRASH_NOW();
        for (auto [entity, mesh] : View<MeshRendererComponent>()) {
            if (!mesh.bvh) {
                mesh.bvh = BvhAccel::Construct(mesh.indices, mesh.positions);
            }
        }
        DVAR_SET_BOOL(gfx_bvh_generate, false);
    }
#endif
}

void Scene::Copy(const Scene& p_other) {
    ComponentId idx = 0;
    for (auto& entry : p_other.m_storage.GetEntries()) {
        if (entry.pool) {
            m_storage.Ensure(idx);
            m_storage.m_entries[idx].pool = std::move(entry.pool->Clone());
        }
        ++idx;
    }

    m_root = p_other.m_root;
    m_bound = p_other.m_bound;
    m_physicsMode = p_other.m_physicsMode;
    m_entity_seed = p_other.m_entity_seed;
}

std::vector<Entity> Scene::GetSortedEntityArray() const {
    std::unordered_set<Entity> entity_set;

    for (const auto& it : m_storage.GetEntries()) {
        if (!it.pool) continue;
        for (auto entity : it.pool->GetEntityArray()) {
            if (Contains<NoSaveTag>(entity)) {
                continue;
            }
            entity_set.insert(entity);
        }
    }

    std::vector<Entity> entity_array(entity_set.begin(), entity_set.end());

    std::sort(entity_array.begin(), entity_array.end());
    return entity_array;
}

void Scene::InstantiatePrefab(PrefabInstanceComponent& p_prefab, ecs::Entity p_ent) {
    auto handle = AssetRegistry::GetSingleton().FindByGuid<Scene>(p_prefab.GetResourceGuid());
    if (handle.is_none()) {
        return;
    }

    const Scene* source = handle.unwrap_unchecked().Get();
    DEV_ASSERT(source);
    Scene copy;
    copy.Copy(*source);

    auto new_entities = copy.GetSortedEntityArray();
    std::unordered_map<Entity, Entity> mapping;
    for (Entity raw_entity : new_entities) {
        Entity mapped = CreateEntity();
        Create<NoSaveTag>(mapped);
        mapping[raw_entity] = mapped;
    }

    // remap hierarchy
    for (auto [id, hier] : copy.View<HierarchyComponent>()) {
        hier.parent_id = mapping[hier.parent_id];
    }

    // remap material
    for (auto [id, renderer] : copy.View<MeshRendererComponent>()) {
        auto& materials = renderer.GetMaterialInstances();
        for (size_t i = 0; i < materials.size(); ++i) {
            materials[i] = mapping[materials[i]];
        }

        CRASH_NOW_MSG("remap skin and skeleton");
    }

    DEV_ASSERT(0);
#if 0
    // remap all entities
    for (auto&& [key, entry] : copy.m_storage.GetEntries()) {
        entry.manager->Remap(mapping);

        auto my_entry = m_component_lib.m_entries.find(key);
        CRASH_COND(my_entry == m_component_lib.m_entries.end());
        my_entry->second.manager->Merge(std::move(*entry.manager));
    }
#endif

    // link instance
    Entity mapped_root = mapping[copy.m_root];
    HierarchyComponent& hier = Create<HierarchyComponent>(mapped_root);
    hier.parent_id = p_ent.IsValid() ? p_ent : m_root;
}

ecs::Entity Scene::FindEntityByName(const char* p_name) {
    for (auto [entity, name] : View<NameComponent>()) {
        if (name.GetName() == p_name) {
            return entity;
        }
    }
    return ecs::Entity::Null();
}

void Scene::AttachChild(ecs::Entity p_child, ecs::Entity p_parent) {
    DEV_ASSERT(p_child != p_parent);
    DEV_ASSERT(p_parent.IsValid());

    // @TODO: prevent circular dependency

    HierarchyComponent* hier = GetComponent<HierarchyComponent>(p_child);

    if (hier == nullptr) {
        hier = &Create<HierarchyComponent>(p_child);
    }

    hier->parent_id = p_parent;
}

template<typename T>
static void DuplicateComponent(Scene& p_scene, ecs::Entity p_source, ecs::Entity p_dest) {
    if (const T* comp = p_scene.GetComponent<T>(p_source)) {
        T copy = *comp;
        T& dest = p_scene.Create<T>(p_dest);
        dest = copy;
    }
}

ecs::Entity Scene::DuplicateEntity(ecs::Entity p_ent) {
    if (!p_ent.IsValid()) {
        return p_ent;
    }

    ecs::Entity entity = CreateEntity();

#define REGISTER_COMPONENT(COMP, ...) DuplicateComponent<COMP>(*this, p_ent, entity);
    REGISTER_COMPONENT_SERIALIZED_LIST
#undef REGISTER_COMPONENT

    return entity;
}

std::vector<Guid> Scene::GetDependencies() const {
    std::vector<Guid> dependencies;
    for (const auto& [id, material] : View<MaterialComponent>()) {
        dependencies.push_back(material.m_material_id);
    }
    for (const auto& [id, mesh_renderer] : View<MeshRendererComponent>()) {
        dependencies.push_back(mesh_renderer.GetResourceGuid());
    }
    for (const auto& [id, prefab] : View<PrefabInstanceComponent>()) {
        dependencies.push_back(prefab.GetResourceGuid());
    }
    for (const auto& [id, tile_map_renderer] : View<TileMapRendererComponent>()) {
        dependencies.push_back(tile_map_renderer.GetResourceGuid());
    }
    for (const auto& [id, animator] : View<SpriteAnimatorComponent>()) {
        dependencies.push_back(animator.GetResourceGuid());
    }

    dependencies.erase(
        std::remove_if(dependencies.begin(), dependencies.end(),
                       [](Guid p_guid) {
                           // @HACK: replace the last two digits to see if guid is 0
                           uint8_t* data = const_cast<uint8_t*>(p_guid.GetData());
                           data[15] = 0;
                           return p_guid.IsNull();
                       }),
        dependencies.end());

    return dependencies;
}

// LATEST_SCENE_VERSION history
// version 1: initial version
// version 2: don't serialize scene.m_bound
// version 3: light component atten
// version 4: light component flags
// version 5: add validation
// version 6: add collider component
// version 7: add enabled to material
// version 8: add particle emitter
// version 9: add ParticleEmitterComponent.gravity
// version 10: add ForceFieldComponent
// version 11: add ScriptFieldComponent
// version 12: add CameraComponent
// version 13: add SoftBodyComponent
// version 14: modify RigidBodyComponent
// version 15: add predefined shadow region to lights
// version 16: change scene binary representation
// version 17: remove armature.flags
// version 18: change RigidBodyComponent
// version 19: serialize scene.m_physicsMode
static constexpr uint32_t LATEST_SCENE_VERSION = 19;
static constexpr char SCENE_MAGIC[] = "xBScene";
static constexpr char SCENE_GUARD_MESSAGE[] = "Should see this message";
static constexpr uint64_t HAS_NEXT_FLAG = 6368519827137030510;

template<typename T>
concept HasOnDeserialized = requires(T& t) {
    { t.OnDeserialized() } -> std::same_as<void>;
};

template<ComponentType T>
static void DeserializeComponent(IDeserializer& d,
                                 const char* p_key,
                                 ecs::Entity p_id,
                                 Scene& p_scene) {
    if (d.TryEnterKey(p_key)) {
        T& component = p_scene.Create<T>(p_id);
        d.Read(component);
        d.LeaveKey();
        if constexpr (HasOnDeserialized<T>) {
            component.OnDeserialized();
        }
    }
}

auto Scene::LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> {
    YAML::Node root;

    if (auto res = LoadYaml(p_meta.import_path, root); !res) {
        return CAVE_ERROR(res.error());
    }

    YamlDeserializer yaml;
    yaml.Initialize(root);

    IDeserializer& d = yaml;

    const int version = d.GetVersion();
    DEV_ASSERT(version);

    if (d.TryEnterKey("seed")) {
        d.Read(m_entity_seed);
        d.LeaveKey();
    }
    if (d.TryEnterKey("root")) {
        d.Read(m_root);
        d.LeaveKey();
    }
    if (d.TryEnterKey("physics_mode")) {
        d.Read(m_physicsMode);
        d.LeaveKey();
    }

    const bool ok = d.TryEnterKey("entities");
    DEV_ASSERT(ok);

    const int entity_count = d.ArraySize().unwrap_or(0);
    for (int i = 0; i < entity_count; ++i) {
        DEV_ASSERT(d.TryEnterIndex(i));
        auto keys = d.GetKeys().unwrap();
        ecs::Entity id;
        DEV_ASSERT(d.TryEnterKey("id"));
        d.Read((uint32_t&)id);
        d.LeaveKey();

#define REGISTER_COMPONENT(a, ...)                 \
    do {                                           \
        DeserializeComponent<a>(d, #a, id, *this); \
    } while (0);
        REGISTER_COMPONENT_SERIALIZED_LIST
#undef REGISTER_COMPONENT

        d.LeaveIndex();
    }

    d.LeaveKey();

    // @TODO: instantiate prefab
    for (auto&& [id, prefab] : View<PrefabInstanceComponent>()) {
        InstantiatePrefab(prefab, id);
    }

    return Result<void>();
}

template<ComponentType T>
static bool SerializeComponent(ISerializer& p_serializer,
                               const char* p_name,
                               ecs::Entity p_ent,
                               const Scene& p_scene) {

    const T* component = p_scene.GetComponent<T>(p_ent);
    if (component) {
        p_serializer.Key(p_name);
        p_serializer.Write(*component);
    }
    return true;
}

auto Scene::SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> {
    auto res = p_meta.SaveToDisk(this);
    if (!res) {
        return CAVE_ERROR(res.error());
    }

    // @TODO: maybe pass ISerializer next
    YamlSerializer yaml;

    auto entity_array = GetSortedEntityArray();

    yaml.BeginMap(false)
        .Key("version")
        .Write(LATEST_SCENE_VERSION)
        .Key("seed")
        .Write(entity_array.back())
        .Key("root")
        .Write(m_root)
        .Key("physics_mode")
        .Write(static_cast<uint32_t>(m_physicsMode))  // @TODO: refactor
        .Key("entities");

    yaml.BeginArray(false);

    for (auto entity : entity_array) {
        if (Contains<NoSaveTag>(entity)) {
            continue;
        }

        yaml.BeginMap(false)
            .Key("id")
            .Write(entity);

#define REGISTER_COMPONENT(COMPONENT, ...) \
    SerializeComponent<COMPONENT>(yaml, #COMPONENT, entity, *this);

        REGISTER_COMPONENT_SERIALIZED_LIST
#undef REGISTER_COMPONENT

        yaml.EndMap();
    }

    yaml.EndArray();
    yaml.EndMap();
    return SaveYaml(p_meta.import_path, yaml);
}

}  // namespace cave
