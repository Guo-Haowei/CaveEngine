#include "cave/core/reflection/Meta.h"
#include "cave/core/reflection/MetaRegistry.h"
#include "cave/runtime/ui/UIComponents.h"

#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/scene/Scene.h"
namespace cave {

#define DEBUG_COMPONENT_REGISTRY NOT_IN_USE
#if USING(DEBUG_COMPONENT_REGISTRY)
#define DEBUG_PRINT(...) LOG_INFO(LogChannel::Core, __VA_ARGS__)
#else
#define DEBUG_PRINT(...) ((void)0)
#endif

const FieldMetaBase* MetaTable::find(const PropertyId& property_type) const {
    for (const FieldMetaBase& meta : props) {
        if (meta.id == property_type) {
            return &meta;
        }
    }
    return nullptr;
}

void MetaRegistry::registerMeta(const MetaTable& meta) {
    const size_t idx = m_table.size();
    auto [it, inserted] = m_lookup.try_emplace(meta.type_id, idx);

    if (!inserted) {
        LOG_FATAL(LogChannel::Core,
                  "meta '{}'(id:{}) already registered",
                  meta.name, meta.type_id.hash());
        return;
    }

    m_table.resize(idx + 1);
    m_table[idx] = meta;

    DEBUG_PRINT("Registered component '{}', id: {}",
                meta.name,
                meta.type_id.hash());
}

const MetaTable* MetaRegistry::tryGet(StringId type_id) const {
    auto it = m_lookup.find(type_id);
    if (it == m_lookup.end()) return nullptr;

    const size_t idx = it->second;
    if (DEV_VERIFY(idx < m_table.size())) {
        return &m_table[idx];
    }
    return nullptr;
}

void MetaRegistry::builtin(MetaRegistry& out) {
#define REGISTER_COMPONENT(T, ...)              \
    out.registerMeta({                          \
        .type_id = T##_Id,                      \
        .name = #T,                             \
        .size = sizeof(T),                      \
        .align = alignof(T),                    \
        .version = 0,                           \
        .props = MetaDataTable<T>::getFields(), \
    });

    // register components
    REGISTER_COMPONENT_SERIALIZED_LIST
#undef REGISTER_COMPONENT
}

}  // namespace cave
