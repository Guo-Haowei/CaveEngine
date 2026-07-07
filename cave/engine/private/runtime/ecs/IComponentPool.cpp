#include "cave/runtime/ecs/IComponentPool.h"

namespace cave::ecs {

bool IComponentPool::has(Entity ent) const {
    if (m_lookup.empty()) {
        return false;
    }
    return m_lookup.find(ent) != m_lookup.end();
}

void IComponentPool::remap(const std::unordered_map<Entity, Entity>& map) {
    std::unordered_map<Entity, size_t> new_lookup;

    for (Entity& entity : m_entity_array) {
        auto it = map.find(entity);
        CRASH_COND_MSG(it == map.end(), "invalid mapping");
        entity = it->second;
    }

    for (const auto& [entity, index] : m_lookup) {
        auto it = map.find(entity);
        CRASH_COND_MSG(it == map.end(), "invalid mapping");
        new_lookup[it->second] = index;
    }

    m_lookup = std::move(new_lookup);
}

}  // namespace cave::ecs
