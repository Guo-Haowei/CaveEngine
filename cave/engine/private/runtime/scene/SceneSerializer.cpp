#include "SceneSerializer.h"

#include "engine/private/runtime/assets/PrefabAsset.h"
#include "engine/private/runtime/assets/SceneAsset.h"
#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/serialization/YamlInclude.h"

namespace cave {

using cave::ecs::Entity;

namespace {

// kLatestSceneVersion history
// version 1:  initial version
// version 2:  don't serialize scene.m_bound
// version 3:  light component atten
// version 4:  light component flags
// version 5:  add validation
// version 6:  add collider component
// version 7:  add enabled to material
// version 8:  add particle emitter
// version 9:  add ParticleEmitterComponent.gravity
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
// version 20: root must have a hier component
constexpr uint32_t kLatestSceneVersion = SceneAsset::kVersion;

#define PREFAB_OVERRIDE_LIST               \
    PREFAB_OVERRIDE(NameComponent)         \
    PREFAB_OVERRIDE(TransformComponent)    \
    PREFAB_OVERRIDE(NativeScriptComponent) \
    PREFAB_OVERRIDE(FacingComponent)

template<typename T>
concept HasOnDeserialized = requires(T& t) {
    { t.OnDeserialized() } -> std::same_as<void>;
};

template<ComponentType T>
Option<T> DeserializeComponent(IDeserializer& d,
                               const char* key) {
    if (d.tryEnterKey(key)) {
        T component;
        d.read(component);
        d.leaveKey();
        if constexpr (HasOnDeserialized<T>) {
            component.OnDeserialized();
        }
        return Some(std::move(component));
    }
    return None();
}

template<ComponentType T>
void DeserializeComponent(IDeserializer& d,
                          const char* key,
                          ecs::Entity ent,
                          Scene& scene) {
    if (d.tryEnterKey(key)) {
        T& component = scene.create<T>(ent);
        d.read(component);
        d.leaveKey();
        if constexpr (HasOnDeserialized<T>) {
            component.OnDeserialized();
        }
    }
}

template<ComponentType T>
bool SerializeComponent(ISerializer& s,
                        const char* name,
                        const Scene& scene,
                        ecs::Entity ent) {

    const T* component = scene.component<T>(ent);
    if (component) {
        s.beginKey(name);
        s.write(*component);
    }
    return true;
}

bool SerializeEntityImpl(ISerializer& s,
                         const Scene& scene,
                         Entity ent) {

    s.beginKey("id")
        .write(ent);

#define REGISTER_COMPONENT(COMPONENT, ...) \
    SerializeComponent<COMPONENT>(s, #COMPONENT, scene, ent);

    REGISTER_COMPONENT_SERIALIZED_LIST
#undef REGISTER_COMPONENT

    return true;
}

bool SerializeNormalEntity(ISerializer& s,
                           const Scene& scene,
                           Entity ent) {

    s.beginMap(false);
    SerializeEntityImpl(s, scene, ent);
    s.endMap();
    return true;
}

template<ComponentType T>
bool SerializeComponentOverride(ISerializer& s,
                                const char* name,
                                const Scene& prefab_scene,
                                Entity prefab_ent,
                                const Scene& instance_scene,
                                Entity instance_ent) {
    const T* prefab_component = prefab_scene.component<T>(prefab_ent);
    const T* instance_component = instance_scene.component<T>(instance_ent);
    if (!prefab_component || !instance_component) {
        return true;
    }

    if (*prefab_component != *instance_component) {
        s.beginKey(name);
        s.write(*instance_component);
    }

    return true;
}

bool SerializePrefabDiff(ISerializer& s,
                         const Scene& scene,
                         Entity ent,
                         const PrefabInstanceComponent& prefab,
                         AssetRegistry* asset_reg) {
    if (!asset_reg) {
        return false;
    }

    auto handle = asset_reg->findByGuid<PrefabAsset>(prefab.prefabGuid());
    if (handle.is_none()) {
        return false;
    }

    const PrefabAsset* prefab_asset = handle.unwrap_unchecked().get();
    DEV_ASSERT(prefab_asset);
    Entity prefab_root = prefab_asset->scene().root();

    s.beginKey("PrefabOverride");
    s.beginMap(false);

#define PREFAB_OVERRIDE(T) \
    SerializeComponentOverride<T>(s, #T, prefab_asset->scene(), prefab_root, scene, ent);
    PREFAB_OVERRIDE_LIST
#undef PREFAB_OVERRIDE

    s.endMap();
    return true;
}

bool SerializePrefabEntity(ISerializer& s,
                           const Scene& scene,
                           Entity ent,
                           const PrefabInstanceComponent& prefab,
                           AssetRegistry* asset_reg) {
    s.beginMap(false);

    s.beginKey("id").write(ent);

    SerializeComponent<HierarchyComponent>(s, "HierarchyComponent", scene, ent);
    SerializeComponent<PrefabInstanceComponent>(s, "PrefabInstanceComponent", scene, ent);

    SerializePrefabDiff(s, scene, ent, prefab, asset_reg);

    s.endMap();
    return true;
}

bool SerializeEntity(ISerializer& s,
                     const Scene& scene,
                     Entity ent,
                     AssetRegistry* asset_reg,
                     bool skip_prefab) {
    if (auto prefab = scene.component<PrefabInstanceComponent>(ent)) {
        return SerializePrefabEntity(s, scene, ent, *prefab, asset_reg);
    }

    if (skip_prefab && scene.has<PrefabChildComponent>(ent)) {
        return true;  // skip prefab entities
    }

    return SerializeNormalEntity(s, scene, ent);
}

}  // namespace

void SerializeScene(ISerializer& s, const Scene& scene, AssetRegistry* asset_reg, bool skip_prefab) {
    auto entity_array = scene.getSortedEntityArray();

    s.beginMap(false)
        .beginKey("version")
        .write(kLatestSceneVersion)
        .beginKey("seed")
        .write(entity_array.back())
        .beginKey("root")
        .write(scene.root())
        .beginKey("entities");

    s.beginArray(false);

    // @TODO: remap entities to use a more contact id

    for (auto ent : entity_array) {
        SerializeEntity(s, scene, ent, asset_reg, skip_prefab);
    }

    s.endArray();

    s.endMap();
}

void DeserializeScene(IDeserializer& d, Scene& scene) {
    const int version = d.version();
    DEV_ASSERT(version);

    if (d.tryEnterKey("seed")) {
        uint32_t seed;
        if (d.read(seed)) {
            scene.setSeed(seed);
        }
        d.leaveKey();
    }
    if (d.tryEnterKey("root")) {
        Entity root;
        if (d.read(root)) {
            scene.setRoot(root);
        }
        d.leaveKey();
    }
    if (d.tryEnterKey("version")) {
        d.read(scene.version());
        d.leaveKey();
    }

    const bool ok = d.tryEnterKey("entities");
    DEV_ASSERT(ok);

    struct OverrideComponents {
        Option<NameComponent> NameComponent;
        Option<TransformComponent> TransformComponent;
        Option<FacingComponent> FacingComponent;
        Option<NativeScriptComponent> NativeScriptComponent;

        bool is_some() const {
            return NameComponent.is_some() ||
                   TransformComponent.is_some() ||
                   FacingComponent.is_some() ||
                   NativeScriptComponent.is_some();
        }
    };

    std::unordered_map<Entity, OverrideComponents> overrides_map;

    const int entity_count = d.arraySize().unwrap_or(0);
    for (int i = 0; i < entity_count; ++i) {
        DEV_ASSERT(d.tryEnterIndex(i));
        auto keys = d.getKeys().unwrap();
        ecs::Entity ent;
        DEV_ASSERT(d.tryEnterKey("id"));
        d.read((uint32_t&)ent);
        d.leaveKey();

        // @TODO: use component registry instead of this
#define REGISTER_COMPONENT(a, ...)                  \
    do {                                            \
        DeserializeComponent<a>(d, #a, ent, scene); \
    } while (0);
        REGISTER_COMPONENT_SERIALIZED_LIST
#undef REGISTER_COMPONENT

        // parse overrides
        OverrideComponents overrides;
        if (d.tryEnterKey("PrefabOverride")) {
#define PREFAB_OVERRIDE(T) overrides.T = DeserializeComponent<T>(d, #T);
            PREFAB_OVERRIDE_LIST
#undef PREFAB_OVERRIDE

            d.leaveKey();
            if (overrides.is_some()) {
                overrides_map[ent] = overrides;
            }
        }

        d.leaveIndex();
    }

    d.leaveKey();

    for (auto&& [ent, prefab] : scene.view<PrefabInstanceComponent>()) {
        InstantiatePrefab(scene, prefab, ent);

        if (!scene.has<HierarchyComponent>(ent)) {
            scene.create<HierarchyComponent>(ent).parent_id = scene.root();
        }
    }

    for (auto&& [ent, overrides] : overrides_map) {
#define PREFAB_OVERRIDE(T)                                         \
    if (overrides.T.is_some()) {                                   \
        *scene.component<T>(ent) = overrides.T.unwrap_unchecked(); \
    }
        PREFAB_OVERRIDE_LIST
#undef PREFAB_OVERRIDE
    }
}

void InstantiatePrefab(Scene& scene, PrefabInstanceComponent& prefab, ecs::Entity parent) {
    DEV_ASSERT(parent.valid());

    // @TODO: remove this
    auto handle_opt = AssetRegistry::singleton().findByGuid<PrefabAsset>(prefab.prefabGuid());
    if (handle_opt.is_none()) {
        return;
    }

    const PrefabAsset* asset = handle_opt.unwrap_unchecked().get();
    DEV_ASSERT(asset);
    Scene copy;
    copy.copy(asset->scene());

    // @TODO: add components to parent, instead of link it
    auto new_entities = copy.getSortedEntityArray();
    std::unordered_map<Entity, Entity> mapping;

    for (Entity prefab_ent : new_entities) {
        if (prefab_ent == copy.root()) {
            mapping[prefab_ent] = parent;
        } else {
            Entity mapped = scene.createEntity();
            scene.create<PrefabChildComponent>(mapped);
            mapping[prefab_ent] = mapped;
        }
    }

    // remap hierarchy
    for (auto [id, hier] : copy.view<HierarchyComponent>()) {
        auto it = mapping.find(hier.parent_id);
        DEV_ASSERT(it != mapping.end());
        hier.parent_id = it->second;
    }

    // remap material
    for (auto [id, renderer] : copy.view<MeshRendererComponent>()) {
        auto& materials = renderer.GetMaterialInstances();
        for (size_t i = 0; i < materials.size(); ++i) {
            materials[i] = mapping[materials[i]];
        }

        CRASH_NOW_MSG("remap skin and skeleton");
    }

    // merge components
    for (uint16_t cid = 0; cid < (uint16_t)copy.storage().entries().size(); ++cid) {
        auto& entry = copy.storage().entries()[cid];
        if (!entry.pool) continue;
        entry.pool->remap(mapping);

        CRASH_COND(cid >= scene.storage().entries().size());
        auto& my_entry = scene.storage().entries()[cid];

        if (!my_entry.pool) {
            scene.storage().getOrCreate(cid);
        }
        my_entry.pool->merge(std::move(*entry.pool));
    }
}

}  // namespace cave
