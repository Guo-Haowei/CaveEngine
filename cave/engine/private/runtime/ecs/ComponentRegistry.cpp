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

    if (m_table.size() <= idx) {
        m_table.resize(idx + 1);
        m_present.resize(idx + 1, 0);
    }

    if (m_present[idx]) {
        LOG_FATAL("ComponentRegistry::Register: component '{}'(id:{}) already registered",
                  p_meta.name, p_meta.id);
        return;
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

ComponentMeta& ComponentRegistry::GetMut(ComponentId p_id) {
    DEV_ASSERT_INDEX(p_id, m_present.size());
    return m_table[p_id];
}

}  // namespace cave::ecs
