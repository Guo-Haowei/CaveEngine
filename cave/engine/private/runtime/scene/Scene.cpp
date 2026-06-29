#include "Scene.h"

#include "cave/core/diagnostics/Profiler.h"
#include "cave/core/threading/JobSystem.h"
#include "cave/runtime/scene/MotorSystem.h"
#include "cave/runtime/script/native/NativeScriptSystem.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"

#include "engine/private/core/io/archive.h"
#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/Engine.h"
#include "engine/private/runtime/scene/SystemManager.h"
#include "engine/private/systems/AnimationSystem.h"
#include "engine/private/systems/EcsSystems.h"

// systems
#include "engine/private/runtime/script/lua/LuaScriptSystem.h"

// @TODO: refactor
#include "engine/private/serialization/yaml_include.h"

namespace cave {

using namespace ::cave::math;
using ecs::Entity;

Scene::Scene(std::string name, ecs::ComponentRegistry& reg) noexcept
    : name_(std::move(name))
    , component_registry_(reg) {
}

Scene::Scene(std::string name) noexcept
    : Scene(std::move(name), engine::GetComponentRegistry()) {
}

Scene::~Scene() = default;

void Scene::update(float dt) {
    CAVE_PROFILE_EVENT();

    m_dirtyFlags.store(0);

    jobsystem::Context ctx;
    // animation
    RunTransformAnimationSystem(*this, ctx, dt);
    RunSpriteAnimationSystem(*this, ctx, dt);
    RunLightUpdateSystem(*this, ctx, dt);
    RunAnimationUpdateSystem(*this, ctx, dt);
    RunFacingUpdateSystem(*this, ctx, dt);
    ctx.Wait();
    // transform, update local matrix from position, rotation and scale
    RunTransformationUpdateSystem(*this, ctx, dt);
    ctx.Wait();
    // hierarchy, update world matrix based on hierarchy
    RunHierarchyUpdateSystem(*this, ctx, dt);
    ctx.Wait();

    RunSkeletonUpdateSystem(*this, ctx, dt);
    ctx.Wait();

    // update bounding box
    RunMeshAABBUpdateSystem(*this, ctx, dt);

    // @TODO: refactor
    for (auto [entity, camera, transform] : view<CameraComponent, TransformComponent>()) {
        if (camera.Update(transform.worldMatrix())) {
            m_dirtyFlags.fetch_or(SCENE_DIRTY_CAMERA);
        }
    }
}

void Scene::copy(const Scene& p_other) {
    ComponentId idx = 0;
    for (auto& entry : p_other.storage_.GetEntries()) {
        if (entry.pool) {
            storage_.Ensure(idx);
            storage_.m_entries[idx].pool = std::move(entry.pool->Clone());
        }
        ++idx;
    }

    m_root = p_other.m_root;
    m_bound = p_other.m_bound;
    entity_seed_ = p_other.entity_seed_;
}

std::vector<Entity> Scene::GetSortedEntityArray() const {
    std::unordered_set<Entity> entity_set;

    for (const auto& it : storage_.GetEntries()) {
        if (!it.pool) continue;
        for (auto entity : it.pool->GetEntityArray()) {
            if (has<NoSaveTag>(entity)) {
                continue;
            }
            entity_set.insert(entity);
        }
    }

    std::vector<Entity> entity_array(entity_set.begin(), entity_set.end());

    std::sort(entity_array.begin(), entity_array.end());
    return entity_array;
}

void Scene::instantiatePrefab(PrefabInstanceComponent& prefab, Entity ent) {
    auto handle = AssetRegistry::singleton().findByGuid<Scene>(prefab.prefabGuid());
    if (handle.is_none()) {
        return;
    }

    const Scene* source = handle.unwrap_unchecked().get();
    DEV_ASSERT(source);
    Scene copy("prefab");
    copy.copy(*source);

    auto new_entities = copy.GetSortedEntityArray();
    std::unordered_map<Entity, Entity> mapping;
    for (Entity raw_entity : new_entities) {
        Entity mapped = createEntity();
        create<NoSaveTag>(mapped);
        mapping[raw_entity] = mapped;
    }

    // remap hierarchy
    for (auto [id, hier] : copy.view<HierarchyComponent>()) {
        hier.parent_id = mapping[hier.parent_id];
    }

    // remap material
    for (auto [id, renderer] : copy.view<MeshRendererComponent>()) {
        auto& materials = renderer.GetMaterialInstances();
        for (size_t i = 0; i < materials.size(); ++i) {
            materials[i] = mapping[materials[i]];
        }

        CRASH_NOW_MSG("remap skin and skeleton");
    }

    // remap all entities
    for (uint16_t cid = 0; cid < (uint16_t)copy.storage_.m_entries.size(); ++cid) {
        auto& entry = copy.storage_.m_entries[cid];
        if (!entry.pool) continue;
        entry.pool->Remap(mapping);

        CRASH_COND(cid >= storage_.m_entries.size());
        auto& my_entry = storage_.m_entries[cid];

        if (!my_entry.pool) {
            storage_.GetOrCreate(cid);
        }
        my_entry.pool->Merge(std::move(*entry.pool));
    }

    // link instance
    Entity mapped_root = mapping[copy.m_root];
    HierarchyComponent& hier = create<HierarchyComponent>(mapped_root);
    hier.parent_id = ent.IsValid() ? ent : m_root;

    TransformComponent* transform = component<TransformComponent>(mapped_root);
    transform->setTranslation(prefab.translation());

    prefab.child(mapped_root);
}

bool Scene::has(ComponentId cid, ecs::Entity ent) const {
    return storage_.Has(ent, cid);
}

size_t Scene::count(ComponentId cid) const {
    if (const ecs::IComponentPool* pool = storage_.TryGet(cid)) {
        return pool->GetCount();
    }

    return 0;
}

bool Scene::remove(ComponentId cid, Entity ent) {
    return storage_.Remove(ent, cid);
}

Entity Scene::findFirstByName(std::string_view name) const {
    for (auto [entity, name_component] : view<NameComponent>()) {
        if (name_component.name() == name) {
            return entity;
        }
    }
    return ecs::Entity::Null();
}

Entity Scene::findChildByName(std::string_view name, Entity ent) const {
    for (auto [entity, hier, name_component] : view<HierarchyComponent, NameComponent>()) {
        if (hier.parent_id == ent && name_component.name() == name) {
            return entity;
        }
    }

    return ecs::Entity::Null();
}

void Scene::removeEntity(ecs::Entity p_ent) {
    // @TODO: move it to SceneCommandExecutor
    if (!p_ent.IsValid()) return;
    std::vector<ecs::Entity> children;
    for (auto [child, hierarchy] : view<HierarchyComponent>()) {
        if (hierarchy.parent_id == p_ent) {
            children.emplace_back(child);
        }
    }

    for (auto child : children) {
        removeEntity(child);
    }

    for (auto& e : storage_.GetEntries()) {
        if (e.pool) {
            e.pool->Remove(p_ent);
        }
    }
}

void Scene::attachChild(ecs::Entity p_child, ecs::Entity p_parent) {
    DEV_ASSERT(p_child != p_parent);
    DEV_ASSERT(p_parent.IsValid());

    // @TODO: prevent circular dependency

    HierarchyComponent* hier = component<HierarchyComponent>(p_child);

    if (hier == nullptr) {
        hier = &create<HierarchyComponent>(p_child);
    }

    hier->parent_id = p_parent;
}

template<typename T>
static void DuplicateComponent(Scene& p_scene, ecs::Entity p_source, ecs::Entity p_dest) {
    if (const T* comp = p_scene.component<T>(p_source)) {
        T copy = *comp;
        T& dest = p_scene.create<T>(p_dest);
        dest = copy;
    }
}

ecs::Entity Scene::duplicateEntity(ecs::Entity p_ent) {
    if (!p_ent.IsValid()) {
        return p_ent;
    }

    ecs::Entity entity = createEntity();

#define REGISTER_COMPONENT(COMP, ...) DuplicateComponent<COMP>(*this, p_ent, entity);
    REGISTER_COMPONENT_SERIALIZED_LIST
#undef REGISTER_COMPONENT

    return entity;
}

std::vector<Guid> Scene::GetDependencies() const {
    std::vector<Guid> dependencies;
    for (const auto& [id, material] : view<MaterialComponent>()) {
        dependencies.push_back(material.m_material_id);
    }
    for (const auto& [id, mesh_renderer] : view<MeshRendererComponent>()) {
        dependencies.push_back(mesh_renderer.GetResourceGuid());
    }
    for (const auto& [id, prefab] : view<PrefabInstanceComponent>()) {
        dependencies.push_back(prefab.prefabGuid());
    }
    for (const auto& [id, tile_map_renderer] : view<TileMapInstanceComponent>()) {
        dependencies.push_back(tile_map_renderer.GetResourceGuid());
    }
    for (const auto& [id, animator] : view<SpriteAnimatorComponent>()) {
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
        T& component = p_scene.create<T>(p_id);
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
        d.Read(entity_seed_);
        d.LeaveKey();
    }
    if (d.TryEnterKey("root")) {
        d.Read(m_root);
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

        // @TODO: use component registry instead of this
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
    for (auto&& [id, prefab] : view<PrefabInstanceComponent>()) {
        instantiatePrefab(prefab, id);
    }

    return Result<void>();
}

template<ComponentType T>
static bool SerializeComponent(ISerializer& p_serializer,
                               const char* p_name,
                               ecs::Entity p_ent,
                               const Scene& p_scene) {

    const T* component = p_scene.component<T>(p_ent);
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
        .Key("entities");

    yaml.BeginArray(false);

    for (auto entity : entity_array) {
        if (has<NoSaveTag>(entity)) {
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

void Scene::onSimBegin(SceneContext& ctx) {
    systems_ = std::make_unique<SystemManager>();

    if (count<NativeScriptComponent>()) {
        systems_->add<NativeScriptSystem>();
    }
    if (count<LuaScriptComponent>()) {
        systems_->add<LuaScriptSystem>();
    }
    if (count<TileMapInstanceComponent>()) {
        systems_->add<TileWorldSystem>();
    }
    if (count<MotorComponent>()) {
        systems_->add<MotorSystem>();
    }

    systems_->onSceneCreate(ctx);
}

void Scene::onSimEnd() {
    systems_->onSceneDestroy();

    systems_.reset();
}

void Scene::simulate(float dt) {
    systems_->update(dt);
}

}  // namespace cave
