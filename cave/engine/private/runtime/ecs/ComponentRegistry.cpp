#include "cave/core/reflection/Meta.h"
#include "cave/runtime/ecs/ComponentRegistry.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/ui/UIComponents.h"

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

const FieldMetaBase* ComponentMeta::find(const PropertyId& pid) const {
    for (const FieldMetaBase* meta : props) {
        if (meta->id == pid) {
            return meta;
        }
    }
    return nullptr;
}

void ComponentRegistry::registerMeta(const ComponentMeta& meta) {
    const size_t idx = meta.cid;

    if (m_table.size() <= idx) {
        m_table.resize(idx + 1);
        m_present.resize(idx + 1, 0);
    }

    if (m_present[idx]) {
        LOG_FATAL("ComponentRegistry::Register: component '{}'(id:{}) already registered",
                  meta.name, meta.cid);
        return;
    }
    DEV_ASSERT(m_present[idx] == 0);
    m_table[idx] = meta;
    m_present[idx] = 1;

    DEBUG_PRINT("Registered component '{}', id: {}", meta.name, meta.cid);
}

const ComponentMeta* ComponentRegistry::tryGet(ComponentId pid) const {
    const size_t idx = (size_t)pid;
    if (idx >= m_present.size() || m_present[idx] == 0) return nullptr;
    return &m_table[idx];
}

ComponentMeta& ComponentRegistry::getMut(ComponentId pid) {
    DEV_ASSERT_INDEX(pid, m_present.size());
    return m_table[pid];
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

}  // namespace

void ComponentRegistry::builtin(ComponentRegistry& out) {
#define REGISTER_COMPONENT(T, ...)              \
    out.registerMeta({                          \
        .cid = T##_Id,                          \
        .name = #T,                             \
        .size = sizeof(T),                      \
        .align = alignof(T),                    \
        .version = 0,                           \
        .props = MetaDataTable<T>::GetFields(), \
    });

    REGISTER_COMPONENT_SERIALIZED_LIST
#undef REGISTER_COMPONENT

    out.getMut(TransformComponent_Id).on_edited = Transform_OnEdited;
}

}  // namespace cave::ecs
