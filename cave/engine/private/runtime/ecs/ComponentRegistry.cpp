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

static void Transform_OnEdited(Scene& p_scene,
                               ecs::Entity p_ent,
                               ComponentId,
                               const PropertyId&,
                               const void*,
                               uint32_t) {
    auto* t = (TransformComponent*)p_scene.storage().GetRaw(p_ent, TransformComponent_Id);
    if (DEV_VERIFY(t)) {
        t->setDirty();
    }
}

static void MeshRenderer_OnEdited(Scene& p_scene,
                                  ecs::Entity p_ent,
                                  ComponentId,
                                  const PropertyId& p_prop_id,
                                  const void*,
                                  uint32_t) {
    if (p_prop_id == "mesh_id"_sid) {
        auto* mesh = (MeshRendererComponent*)p_scene.storage().GetRaw(p_ent, MeshRendererComponent_Id);
        if (DEV_VERIFY(mesh)) {
            mesh->OnDeserialized();
        }
    }
}

static void Materail_OnEdited(Scene& p_scene,
                              ecs::Entity p_ent,
                              ComponentId,
                              const PropertyId& p_prop_id,
                              const void*,
                              uint32_t) {
    if (p_prop_id == "material_id"_sid) {
        auto* m = (MaterialComponent*)p_scene.storage().GetRaw(p_ent, MaterialComponent_Id);
        if (DEV_VERIFY(m)) {
            m->OnDeserialized();
        }
    }
}

void ComponentRegistry::Builtin(ComponentRegistry& p_out) {
#define REGISTER_COMPONENT(T, ...)              \
    p_out.Register({                            \
        .cid = T##_Id,                          \
        .name = #T,                             \
        .size = sizeof(T),                      \
        .align = alignof(T),                    \
        .version = 0,                           \
        .props = MetaDataTable<T>::GetFields(), \
    });

    REGISTER_COMPONENT_SERIALIZED_LIST
#undef REGISTER_COMPONENT

    {
        ecs::ComponentMeta& meta = p_out.GetMut(TransformComponent_Id);
        meta.on_edited = Transform_OnEdited;
    }
    {
        ecs::ComponentMeta& meta = p_out.GetMut(MeshRendererComponent_Id);
        meta.on_edited = MeshRenderer_OnEdited;
    }
    {
        ecs::ComponentMeta& meta = p_out.GetMut(MaterialComponent_Id);
        meta.on_edited = Materail_OnEdited;
    }
}

}  // namespace cave::ecs
