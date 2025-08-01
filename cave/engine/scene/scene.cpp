#include "scene.h"

#include "engine/assets/mesh_asset.h"
#include "engine/core/debugger/profiler.h"
#include "engine/core/io/archive.h"
#include "engine/ecs/component_manager.inl"
#include "engine/runtime/asset_registry.h"
#include "engine/systems/animation_system.h"
#include "engine/systems/ecs_systems.h"
#include "engine/systems/job_system/job_system.h"

// @TODO: refactor
#include "engine/renderer/graphics_dvars.h"
#include "engine/renderer/path_tracer/bvh_accel.h"
#include "engine/serialization/yaml_include.h"

namespace cave::ecs {

// instantiate ComponentManagers
#define REGISTER_COMPONENT(TYPE, ...) template class ComponentManager<::cave::TYPE>;
REGISTER_COMPONENT_LIST
#undef REGISTER_COMPONENT

}  // namespace cave::ecs

namespace cave {

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
    // armature

    RunArmatureUpdateSystem(*this, ctx, p_timestep);
    ctx.Wait();

    // update bounding box
    RunMeshAABBUpdateSystem(*this, ctx, p_timestep);

    // @TODO: refactor
    for (auto [entity, camera] : View<CameraComponent>()) {
        if (camera.Update()) {
            m_dirtyFlags.fetch_or(SCENE_DIRTY_CAMERA);
        }
    }

    for (auto [entity, voxel_gi] : View<VoxelGiComponent>()) {
        auto transform = GetComponent<TransformComponent>(entity);
        if (DEV_VERIFY(transform)) {
            const auto& matrix = transform->GetWorldMatrix();
            Vector3f center{ matrix[3].x, matrix[3].y, matrix[3].z };
            Vector3f scale = transform->GetScale();
            const float size = glm::max(scale.x, glm::max(scale.y, scale.z));
            voxel_gi.region = AABB::FromCenterSize(center, Vector3f(size));
        }
    }

    // @TODO: refactor
#if 0
    if (DVAR_GET_BOOL(gfx_bvh_generate)) {
        CRASH_NOW();
        for (auto [entity, mesh] : m_MeshComponents) {
            if (!mesh.bvh) {
                mesh.bvh = BvhAccel::Construct(mesh.indices, mesh.positions);
            }
        }
        DVAR_SET_BOOL(gfx_bvh_generate, false);
    }
#endif
}

void Scene::Copy(Scene& p_other) {
    for (auto& entry : m_componentLib.m_entries) {
        auto& manager = *p_other.m_componentLib.m_entries[entry.first].manager;
        entry.second.manager->Copy(manager);
    }

    m_root = p_other.m_root;
    m_bound = p_other.m_bound;
    m_physicsMode = p_other.m_physicsMode;
}

void Scene::InstantiatePrefab(ecs::Entity p_entity, PrefabInstanceComponent& p_prefab) {
    if (p_prefab.instantiated) {
        return;
    }

    auto handle = AssetRegistry::GetSingleton().FindByGuid<Scene>(p_prefab.prefab_id);
    if (handle.is_none()) {
        return;
    }
}

void Scene::Merge(Scene& p_other) {
    CRASH_NOW_MSG("SHOULD NOT CALL THIS");
    // @TODO: check the correctness of this
    for (auto& entry : m_componentLib.m_entries) {
        auto& manager = *p_other.m_componentLib.m_entries[entry.first].manager;
        entry.second.manager->Merge(manager);
    }
    if (p_other.m_root.IsValid()) {
        AttachChild(p_other.m_root, m_root);
    }

    m_bound.UnionBox(p_other.m_bound);
}

ecs::Entity Scene::GetMainCamera() {
    for (auto [entity, camera] : View<CameraComponent>()) {
        if (camera.HasPrimaryFlag()) {
            return entity;
        }
    }

    return ecs::Entity::Null();
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

    // if child already has a parent, detach it
    if (Contains<HierarchyComponent>(p_child)) {
        CRASH_NOW_MSG("Unlikely to happen at this point");
    }

    HierarchyComponent& hier = Create<HierarchyComponent>(p_child);
    hier.parent_id = p_parent;
}

void Scene::RemoveEntity(ecs::Entity p_entity) {
    std::vector<ecs::Entity> children;
    for (auto [child, hierarchy] : View<HierarchyComponent>()) {
        if (hierarchy.parent_id == p_entity) {
            children.emplace_back(child);
        }
    }

    for (auto child : children) {
        RemoveEntity(child);
    }

    for (auto&& [_, component_manager] : m_componentLib.m_entries) {
        component_manager.manager->Remove(p_entity);
    }
}

bool Scene::RayObjectIntersect(ecs::Entity p_id, Ray& p_ray) {
    MeshRendererComponent* renderer = GetComponent<MeshRendererComponent>(p_id);
    MeshAsset* mesh = renderer->GetMeshHandle().Get();
    TransformComponent* transform = GetComponent<TransformComponent>(p_id);
    DEV_ASSERT(mesh && transform);

    if (!transform || !mesh) {
        return false;
    }

    Matrix4x4f inversedModel = glm::inverse(transform->GetWorldMatrix());
    Ray inversedRay = p_ray.Inverse(inversedModel);
    Ray inversedRayAABB = inversedRay;  // make a copy, we don't want dist to be modified by AABB
    // Perform aabb test
    if (!inversedRayAABB.Intersects(mesh->localBound)) {
        return false;
    }

    // @TODO: test submesh intersection

    // Test every single triange
    for (size_t i = 0; i < mesh->indices.size(); i += 3) {
        const Vector3f& A = mesh->positions[mesh->indices[i]];
        const Vector3f& B = mesh->positions[mesh->indices[i + 1]];
        const Vector3f& C = mesh->positions[mesh->indices[i + 2]];
#define CC(a) Vector3f(a.x, a.y, a.z)
        if (inversedRay.Intersects(CC(A), CC(B), CC(C))) {
#undef CC
            p_ray.CopyDist(inversedRay);
            return true;
        }
    }
    return false;
}

Scene::RayIntersectionResult Scene::Intersects(Ray& p_ray) {
    RayIntersectionResult result;

    for (auto [entity, _] : View<MeshRendererComponent>()) {
        if (RayObjectIntersect(entity, p_ray)) {
            result.entity = entity;
        }
    }

    return result;
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
        dependencies.push_back(prefab.prefab_id);
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

    return Result<void>();
}

template<ComponentType T>
static bool SerializeComponent(ISerializer& p_serializer,
                               const char* p_name,
                               ecs::Entity p_entity,
                               const Scene& p_scene) {

    const T* component = p_scene.GetComponent<T>(p_entity);
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

    std::unordered_set<uint32_t> entity_set;

    for (const auto& it : m_componentLib.m_entries) {
        auto& manager = it.second.manager;
        for (auto entity : manager->GetEntityArray()) {
            entity_set.insert(entity.GetId());
        }
    }

    std::vector<uint32_t> entity_array(entity_set.begin(), entity_set.end());
    std::sort(entity_array.begin(), entity_array.end());

    yaml.BeginMap(false)
        .Key("version")
        .Write(LATEST_SCENE_VERSION)
        .Key("seed")
        .Write(m_entity_seed)
        .Key("root")
        .Write(m_root.GetId())
        .Key("physics_mode")
        .Write(static_cast<uint32_t>(m_physicsMode))
        .Key("entities");

    yaml.BeginArray(false);

    for (auto id : entity_array) {
        ecs::Entity entity{ id };

        yaml.BeginMap(false)
            .Key("id")
            .Write(id);

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

#if 0
Result<void> SaveSceneBinary(const std::string& p_path, Scene& p_scene) {
    Archive archive;
    if (auto res = archive.OpenWrite(p_path); !res) {
        return CAVE_ERROR(res.error());
    }

    archive << SCENE_MAGIC;
    archive << LATEST_SCENE_VERSION;
    archive << ecs::Entity::GetSeed();
    archive << p_scene.m_root;
    archive << p_scene.m_physicsMode;

    archive << SCENE_GUARD_MESSAGE;

    for (const auto& it : p_scene.GetLibraryEntries()) {
        if (it.second.m_manager->GetCount()) {
            archive << HAS_NEXT_FLAG;
            archive << it.first;  // write name
            it.second.m_manager->Serialize(archive, LATEST_SCENE_VERSION);
        }
    }
    archive << uint64_t(0);
    return Result<void>();
}

Result<void> LoadSceneBinary(const std::string& p_path, Scene& p_scene) {
    Archive archive;
    if (auto res = archive.OpenRead(p_path); !res) {
        return CAVE_ERROR(res.error());
    }

    char magic[sizeof(SCENE_MAGIC)]{ 0 };
    if (!archive.Read(magic) || !StringUtils::StringEqual(magic, SCENE_MAGIC)) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_CORRUPT, "file corrupted, magic is not '{}'", SCENE_MAGIC);
    }

    uint32_t version;
    if (!archive.Read(version) || version > LATEST_SCENE_VERSION) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_CORRUPT, "incorrect scene version {}, current version is {}", version, LATEST_SCENE_VERSION);
    }

    SCENE_DBG_LOG("loading scene '{}', version: {}", p_path, version);

    uint32_t seed = ecs::Entity::MAX_ID;
    if (!archive.Read(seed)) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_CORRUPT, "failed to read seed");
    }

    archive >> p_scene.m_root;

    p_scene.m_physicsMode = PhysicsMode::NONE;
    if (version >= 19) {
        archive >> p_scene.m_physicsMode;
    }

    char guard_message[sizeof(SCENE_GUARD_MESSAGE)]{ 0 };
    archive >> guard_message;
    if (!StringUtils::StringEqual(guard_message, SCENE_GUARD_MESSAGE)) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_CORRUPT);
    }

    for (;;) {
        uint64_t has_next = 0;
        archive >> has_next;
        if (has_next != HAS_NEXT_FLAG) {
            return Result<void>();
        }

        std::string key;
        archive >> key;

        SCENE_DBG_LOG("Loading Component {}", key);

        auto it = p_scene.GetLibraryEntries().find(key);
        if (it == p_scene.GetLibraryEntries().end()) {
            return CAVE_ERROR(ErrorCode::ERR_FILE_CORRUPT, "entry '{}' not found", key);
        }
        if (!it->second.m_manager->Serialize(archive, version)) {
            return CAVE_ERROR(ErrorCode::ERR_FILE_CORRUPT, "failed to serialize '{}'", key);
        }
    }
}
#endif
}  // namespace cave
