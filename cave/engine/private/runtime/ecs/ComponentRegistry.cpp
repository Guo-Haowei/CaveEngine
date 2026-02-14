#include "cave/runtime/ecs/ComponentRegistry.h"

#include "cave/core/reflection/Meta.h"

namespace cave::ecs {

#define DEBUG_COMPONENT_REGISTRY IN_USE
// #define DEBUG_COMPONENT_REGISTRY NOT_IN_USE
#if USING(DEBUG_COMPONENT_REGISTRY)
#define DEBUG_PRINT(...) LOG_VERBOSE(__VA_ARGS__)
#else
#define DEBUG_PRINT(...) ((void)0)
#endif

const FieldMetaBase* ComponentMeta::Find(PropertyId p_id) const {
    for (const FieldMetaBase* meta : props) {
        if (meta->name == p_id) {
            return meta;
        }
    }
    return nullptr;
}

void ComponentRegistry::Register(const ComponentMeta& p_meta) {
    const size_t idx = p_meta.id;

    auto [it, inserted] = m_name_to_id.try_emplace(p_meta.name_id);
    if (!inserted) {
        LOG_FATAL("ComponentRegistry::Register: component '{}'(hash:{}) already registered",
                  p_meta.name,
                  p_meta.name_id.GetHash());
        return;
    }

    if (m_table.size() <= idx) {
        m_table.resize(idx + 1);
        m_present.resize(idx + 1, 0);
    }

    DEV_ASSERT(m_present[idx] == 0);
    m_table[idx] = p_meta;
    m_present[idx] = 1;

    DEBUG_PRINT("Registered component '{}', id: {}", p_meta.name, p_meta.id);
}

const ComponentMeta* ComponentRegistry::TryGet(ComponentId p_id) const {
    const size_t idx = (size_t)p_id;
    if (idx >= m_present.size() || m_present[idx] == 0) return nullptr;
    return &m_table[idx];
}

const ComponentMeta* ComponentRegistry::FindByName(const StringId& p_id) const {
    auto it = m_name_to_id.find(p_id);
    if (it == m_name_to_id.end()) return nullptr;
    return TryGet(it->second);
}

}  // namespace cave::ecs
