#include "cave/runtime/ecs/ComponentStorage.h"

#include "ComponentPool.h"

// @TODO: refactor
#include "engine/private/runtime/framework/Engine.h"

namespace cave::ecs {

void ComponentStorage::ClearAll() {
    for (auto& entry : m_entries) {
        if (entry.pool) {
            entry.pool->Clear();
        }
    }
}

bool ComponentStorage::IsRegistered(ComponentId p_id) const {
    const size_t idx = p_id;
    return idx < m_entries.size() && m_entries[idx].pool != nullptr;
}

IComponentPool* ComponentStorage::TryGet(ComponentId p_id) const {
    const size_t idx = (size_t)p_id;
    if (idx >= m_entries.size()) return nullptr;
    return m_entries[idx].pool.get();
}

IComponentPool* ComponentStorage::FindByName(const StringId& p_id) const {
    auto& reg = engine::GetComponentRegistry();
    if (const ComponentMeta* meta = reg.FindByName(p_id)) {
        return TryGet(meta->id);
    }
    return nullptr;
}

bool ComponentStorage::Has(Entity p_ent, ComponentId p_id) const {
    auto* pool = TryGet(p_id);
    return pool ? pool->Contains(p_ent) : false;
}

void* ComponentStorage::GetRaw(Entity p_ent, ComponentId p_id) {
    auto* pool = TryGet(p_id);
    return pool ? pool->GetRaw(p_ent) : nullptr;
}

void* ComponentStorage::AddDefault(Entity p_ent, ComponentId p_id) {
    auto* pool = TryGet(p_id);
    return pool ? pool->CreateDefaultRaw(p_ent) : nullptr;
}

bool ComponentStorage::Remove(Entity p_ent, ComponentId p_id) {
    auto* pool = TryGet(p_id);
    if (!pool) return false;
    if (!pool->Contains(p_ent)) return true;
    pool->Remove(p_ent);
    return true;
}

void ComponentStorage::Ensure(ComponentId p_id) {
    const size_t need = p_id + 1;
    if (m_entries.size() < need) m_entries.resize(need);
}

}  // namespace cave::ecs
