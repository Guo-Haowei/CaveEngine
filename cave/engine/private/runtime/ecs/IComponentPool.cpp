#include "cave/runtime/ecs/IComponentPool.h"

namespace cave::ecs {

bool IComponentPool::has(Entity p_ent) const {
    if (lookup_.empty()) {
        return false;
    }
    return lookup_.find(p_ent) != lookup_.end();
}

void IComponentPool::remap(const std::unordered_map<Entity, Entity>& p_map) {
    std::unordered_map<Entity, size_t> new_lookup;

    for (Entity& entity : entity_array_) {
        auto it = p_map.find(entity);
        CRASH_COND_MSG(it == p_map.end(), "invalid mapping");
        entity = it->second;
    }

    for (const auto& [entity, index] : lookup_) {
        auto it = p_map.find(entity);
        CRASH_COND_MSG(it == p_map.end(), "invalid mapping");
        new_lookup[it->second] = index;
    }

    lookup_ = std::move(new_lookup);
}

}  // namespace cave::ecs
