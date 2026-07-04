#include "cave/core/reflection/Meta.h"
#include "cave/runtime/ecs/ComponentRegistry.h"

#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave::ecs {

using namespace cave::literals;

#define DEBUG_COMPONENT_REGISTRY NOT_IN_USE
// #define DEBUG_COMPONENT_REGISTRY NOT_IN_USE
#if USING(DEBUG_COMPONENT_REGISTRY)
#define DEBUG_PRINT(...) LOG_VERBOSE(__VA_ARGS__)
#else
#define DEBUG_PRINT(...) ((void)0)
#endif

const FieldMetaBase* ComponentMeta::Find(const PropertyId& p_id) const {
    for (const FieldMetaBase* meta : props) {
        if (meta->id == p_id) {
            return meta;
        }
    }
    return nullptr;
}

void ComponentRegistry::Register(const ComponentMeta& p_meta) {
    const size_t idx = p_meta.cid;

    if (m_table.size() <= idx) {
        m_table.resize(idx + 1);
        m_present.resize(idx + 1, 0);
    }

    if (m_present[idx]) {
        LOG_FATAL("ComponentRegistry::Register: component '{}'(id:{}) already registered",
                  p_meta.name, p_meta.cid);
        return;
    }
    DEV_ASSERT(m_present[idx] == 0);
    m_table[idx] = p_meta;
    m_present[idx] = 1;

    DEBUG_PRINT("Registered component '{}', id: {}", p_meta.name, p_meta.cid);
}

const ComponentMeta* ComponentRegistry::TryGet(ComponentId p_id) const {
    const size_t idx = (size_t)p_id;
    if (idx >= m_present.size() || m_present[idx] == 0) return nullptr;
    return &m_table[idx];
}

ComponentMeta& ComponentRegistry::GetMut(ComponentId p_id) {
    DEV_ASSERT_INDEX(p_id, m_present.size());
    return m_table[p_id];
}

namespace {

void Transform_OnEdited(Scene& scene,
                        ecs::Entity ent,
                        ComponentId,
                        const PropertyId&,
                        const void*,
                        uint32_t) {
    auto* c = (TransformComponent*)scene.storage().getRaw(TransformComponent_Id, ent);
    if (DEV_VERIFY(c)) {
        c->setDirty();
    }
}

void MeshRenderer_OnEdited(Scene& scene,
                           ecs::Entity ent,
                           ComponentId,
                           const PropertyId& pid,
                           const void*,
                           uint32_t) {
    if (pid == "mesh_id"_sid) {
        auto* c = (MeshRendererComponent*)scene.storage().getRaw(MeshRendererComponent_Id, ent);
        if (DEV_VERIFY(c)) {
            c->OnDeserialized();
        }
    }
}

void Materail_OnEdited(Scene& scene,
                       ecs::Entity ent,
                       ComponentId,
                       const PropertyId& pid,
                       const void*,
                       uint32_t) {
    if (pid == "material_id"_sid) {
        auto* c = (MaterialComponent*)scene.storage().getRaw(MaterialComponent_Id, ent);
        if (DEV_VERIFY(c)) {
            c->OnDeserialized();
        }
    }
}

void TileMapInstance_OnEdited(Scene& scene,
                              ecs::Entity ent,
                              ComponentId,
                              const PropertyId& pid,
                              const void*,
                              uint32_t) {
    if (pid == "tile_map_id"_sid) {
        auto* c = (TileMapInstanceComponent*)scene.storage().getRaw(TileMapInstanceComponent_Id, ent);
        if (DEV_VERIFY(c)) {
            c->OnDeserialized();
        }
    }
}

void PrefabInstance_OnEdited(Scene& scene,
                             ecs::Entity ent,
                             ComponentId,
                             const PropertyId& pid,
                             const void* data,
                             uint32_t) {
    if (pid == "prefab_id"_sid) {
        auto* c = (PrefabInstanceComponent*)scene.storage().getRaw(PrefabInstanceComponent_Id, ent);
        if (DEV_VERIFY(c)) {
            const Guid* guid = (const Guid*)data;
            unused(guid);
            scene.instantiatePrefab(*c, ent);
        }
    }
}

}  // namespace

void ComponentRegistry::Builtin(ComponentRegistry& out) {
#define REGISTER_COMPONENT(T, ...)              \
    out.Register({                              \
        .cid = T##_Id,                          \
        .name = #T,                             \
        .size = sizeof(T),                      \
        .align = alignof(T),                    \
        .version = 0,                           \
        .props = MetaDataTable<T>::GetFields(), \
    });

    REGISTER_COMPONENT_SERIALIZED_LIST
#undef REGISTER_COMPONENT

    out.GetMut(TransformComponent_Id).on_edited = Transform_OnEdited;
    out.GetMut(MeshRendererComponent_Id).on_edited = MeshRenderer_OnEdited;
    out.GetMut(MaterialComponent_Id).on_edited = Materail_OnEdited;
    out.GetMut(TileMapInstanceComponent_Id).on_edited = TileMapInstance_OnEdited;
    out.GetMut(PrefabInstanceComponent_Id).on_edited = PrefabInstance_OnEdited;
}

}  // namespace cave::ecs
