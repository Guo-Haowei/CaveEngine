#include "cave/core/reflection/Meta.h"
#include "cave/runtime/ecs/ComponentRegistry.h"
#include "cave/runtime/ui/UIComponents.h"

#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave::ecs {

using namespace cave::literals;

#define DEBUG_COMPONENT_REGISTRY IN_USE
#if USING(DEBUG_COMPONENT_REGISTRY)
#define DEBUG_PRINT(...) LOG_INFO(LogChannel::Core, __VA_ARGS__)
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
    const size_t idx = m_table.size();
    auto [it, inserted] = m_lookup.try_emplace(meta.cid, idx);

    if (!inserted) {
        LOG_FATAL(LogChannel::Core,
                  "meta '{}'(id:{}) already registered",
                  meta.name, meta.cid.hash());
        return;
    }

    m_table.resize(idx + 1);
    m_table[idx] = meta;

    DEBUG_PRINT("Registered component '{}', id: {}",
                meta.name,
                meta.cid.hash());
}

const ComponentMeta* ComponentRegistry::tryGet(ComponentId cid) const {
    auto it = m_lookup.find(cid);
    if (it == m_lookup.end()) return nullptr;

    const size_t idx = it->second;
    if (DEV_VERIFY(idx < m_table.size())) {
        return &m_table[idx];
    }
    return nullptr;
}

ComponentMeta& ComponentRegistry::getMut(ComponentId cid) {
    auto it = m_lookup.find(cid);
    DEV_ASSERT(it != m_lookup.end());

    const size_t idx = it->second;
    DEV_ASSERT(idx < m_table.size());
    return m_table[idx];
}

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
}

}  // namespace cave::ecs
