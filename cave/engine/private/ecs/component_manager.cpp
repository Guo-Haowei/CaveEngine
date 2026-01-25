#include "component_manager.h"

namespace cave::ecs {

void IComponentManager::Remap(const std::unordered_map<Entity, Entity>& p_map) {
    std::unordered_map<Entity, size_t> new_lookup;

    for (Entity& entity : m_entityArray) {
        auto it = p_map.find(entity);
        CRASH_COND_MSG(it == p_map.end(), "invalid mapping");
        entity = it->second;
    }

    for (const auto& [entity, index] : m_lookup) {
        auto it = p_map.find(entity);
        CRASH_COND_MSG(it == p_map.end(), "invalid mapping");
        new_lookup[it->second] = index;
    }

    m_lookup = std::move(new_lookup);
}

}  // namespace cave::ecs
