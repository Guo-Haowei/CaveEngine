#include "scene.h"

#include "engine/core/debugger/profiler.h"
#include "engine/core/io/archive.h"
#include "engine/ecs/component_manager.inl"
#include "engine/runtime/asset_registry.h"
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
    RunMeshEmitterUpdateSystem(*this, ctx, p_timestep);
    // particle
    RunParticleEmitterUpdateSystem(*this, ctx, p_timestep);
    // armature
    RunArmatureUpdateSystem(*this, ctx, p_timestep);
    ctx.Wait();

    // update bounding box
    RunObjectUpdateSystem(*this, ctx, p_timestep);

    // @TODO: refactor
    for (auto [entity, camera] : m_CameraComponents) {
        if (camera.Update()) {
            m_dirtyFlags.fetch_or(SCENE_DIRTY_CAMERA);
        }
    }

    for (auto [entity, voxel_gi] : m_VoxelGiComponents) {
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
    if (DVAR_GET_BOOL(gfx_bvh_generate)) {
        for (auto [entity, mesh] : m_MeshComponents) {
            if (!mesh.bvh) {
                mesh.bvh = BvhAccel::Construct(mesh.indices, mesh.positions);
            }
        }
        DVAR_SET_BOOL(gfx_bvh_generate, false);
    }
}

void Scene::Copy(Scene& p_other) {
    for (auto& entry : m_componentLib.m_entries) {
        auto& manager = *p_other.m_componentLib.m_entries[entry.first].m_manager;
        entry.second.m_manager->Copy(manager);
    }

    m_root = p_other.m_root;
    m_bound = p_other.m_bound;
    m_physicsMode = p_other.m_physicsMode;
}

void Scene::Merge(Scene& p_other) {
    for (auto& entry : m_componentLib.m_entries) {
        auto& manager = *p_other.m_componentLib.m_entries[entry.first].m_manager;
        entry.second.m_manager->Merge(manager);
    }
    if (p_other.m_root.IsValid()) {
        AttachChild(p_other.m_root, m_root);
    }

    m_bound.UnionBox(p_other.m_bound);
}

ecs::Entity Scene::GetMainCamera() {
    for (auto [entity, camera] : m_CameraComponents) {
        if (camera.IsPrimary()) {
            return entity;
        }
    }

    return ecs::Entity::INVALID;
}

ecs::Entity Scene::FindEntityByName(const char* p_name) {
    for (auto [entity, name] : m_NameComponents) {
        if (name.GetName() == p_name) {
            return entity;
        }
    }
    return ecs::Entity::INVALID;
}

void Scene::AttachChild(ecs::Entity p_child, ecs::Entity p_parent) {
    DEV_ASSERT(p_child != p_parent);
    DEV_ASSERT(p_parent.IsValid());

    // if child already has a parent, detach it
    if (Contains<HierarchyComponent>(p_child)) {
        CRASH_NOW_MSG("Unlikely to happen at this point");
    }

    HierarchyComponent& hier = Create<HierarchyComponent>(p_child);
    hier.m_parent_id = p_parent;
}

void Scene::RemoveEntity(ecs::Entity p_entity) {
    std::vector<ecs::Entity> children;
    for (auto [child, hierarchy] : m_HierarchyComponents) {
        if (hierarchy.GetParent() == p_entity) {
            children.emplace_back(child);
        }
    }
    for (auto child : children) {
        RemoveEntity(child);
    }

    LightComponent* light = GetComponent<LightComponent>(p_entity);
    if (light) {
        // @TODO: shadow atlas
        m_LightComponents.Remove(p_entity);
    }
    m_HierarchyComponents.Remove(p_entity);
    m_TransformComponents.Remove(p_entity);
    m_MeshRenderers.Remove(p_entity);
    m_ParticleEmitterComponents.Remove(p_entity);
    m_ForceFieldComponents.Remove(p_entity);
    m_NameComponents.Remove(p_entity);
}

bool Scene::RayObjectIntersect(ecs::Entity p_object_id, Ray& p_ray) {
    MeshRenderer* object = GetComponent<MeshRenderer>(p_object_id);
    MeshComponent* mesh = GetComponent<MeshComponent>(object->meshId);
    TransformComponent* transform = GetComponent<TransformComponent>(p_object_id);
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

    // @TODO: box collider
    for (size_t object_idx = 0; object_idx < GetCount<MeshRenderer>(); ++object_idx) {
        ecs::Entity entity = GetEntity<MeshRenderer>(object_idx);
        if (RayObjectIntersect(entity, p_ray)) {
            result.entity = entity;
        }
    }

    return result;
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

#if 0
template<ComponentType T>
static Result<void> DeserializeComponent(const YAML::Node& p_node,
                                         const char* p_key,
                                         ecs::Entity p_id,
                                         uint32_t p_version,
                                         Scene& p_scene,
                                         FileAccess* p_binary) {
    const auto& node = p_node[p_key];
    if (!node.IsDefined()) {
        return Result<void>();
    }

    if (!node.IsMap()) {
        return CAVE_ERROR(ErrorCode::ERR_PARSE_ERROR, "entity {} has invalid '{}'", p_id.GetId(), p_key);
    }

    unused(p_scene);
    unused(p_version);
    unused(p_binary);
    unused(p_id);

    // @TODO: reserve component manager
    T& component = p_scene.Create<T>(p_id);
    YamlSerializer context;
    context.file = p_binary;
    context.version = p_version;
    if (auto res = DeserializeYaml(node, component, context); !res) {
        return CAVE_ERROR(res.error());
    }

    component.OnDeserialized();
    return Result<void>();
}
#endif

auto Scene::LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> {
    YAML::Node root;

    if (auto res = LoadYaml(p_meta.path, root); !res) {
        return CAVE_ERROR(res.error());
    }

    YamlDeserializer yaml;
    yaml.Initialize(root);

    IDeserializer& d = yaml;

    const int version = d.GetVersion();
    DEV_ASSERT(version);

    if (d.TryEnterKey("seed")) {
        uint32_t seed;
        d.Read(seed);
        ecs::Entity::SetSeed(seed);
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
        if (d.TryEnterIndex(i)) {

    for (const auto& entity : entities) {
        if (!entity.IsMap()) {
            return CAVE_ERROR(ErrorCode::ERR_FILE_CORRUPT, "invalid format");
        }

        ecs::Entity id(entity["id"].as<uint32_t>());

#define REGISTER_COMPONENT(a, ...)                                                         \
    do {                                                                                   \
        auto res2 = DeserializeComponent<a>(entity, #a, id, version, p_scene, file.get()); \
        if (!res2) { return CAVE_ERROR(res2.error()); }                                    \
    } while (0);
        REGISTER_COMPONENT_SERIALIZED_LIST
#undef REGISTER_COMPONENT
    }

            // @TODO: read components
            d.LeaveIndex();
        }
    }

    d.LeaveKey();

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
        auto& manager = it.second.m_manager;
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
        .Write(ecs::Entity::GetSeed())
        .Key("root")
        .Write(m_root.GetId())
        .Key("physics_mode")
        .Write(static_cast<uint32_t>(m_physicsMode))
        .Key("entities");

    yaml.BeginArray(false);

    // bool ok = true;
    // ok = ok && archive.Write(SCENE_MAGIC);
    // ok = ok && archive.Write(LATEST_SCENE_VERSION);
    // ok = ok && archive.Write(SCENE_GUARD_MESSAGE);
    // if (!ok) {
    //     return CAVE_ERROR(ErrorCode::ERR_FILE_CANT_WRITE, "failed to save file '{}'", p_path);
    // }

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
    return SaveYaml(p_meta.path, yaml);
}

}  // namespace cave
