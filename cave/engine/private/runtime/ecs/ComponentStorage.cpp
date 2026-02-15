#include "cave/runtime/ecs/ComponentStorage.h"

#include "ComponentPool.h"

// @TODO: refactor
#include "engine/private/runtime/framework/Engine.h"
#include "engine/private/runtime/ecs/components/All.h"

namespace cave::ecs {

void ComponentStorage::ClearAll() {
    for (auto& entry : m_entries) {
        if (entry.pool) {
            entry.pool->Clear();
        }
    }
}

IComponentPool& ComponentStorage::GetOrCreate(ComponentId p_id) {
    Ensure(p_id);
    Entry& e = m_entries[p_id];

    // @TODO: instead of this, make pool typeless
    if (!e.pool) {
        switch (p_id) {
#define REGISTER_COMPONENT(TYPE, ...)                     \
    case TYPE##_Id: {                                     \
        e.pool = std::make_unique<ComponentPool<TYPE>>(); \
    } break;
            REGISTER_COMPONENT_LIST
#undef REGISTER_COMPONENT
            default:
                CRASH_NOW();
                break;
        }
    }

    return *e.pool;
}

bool ComponentStorage::IsRegistered(ComponentId p_id) const {
    const size_t idx = p_id;
    return idx < m_entries.size() && m_entries[idx].pool != nullptr;
}

IComponentPool* ComponentStorage::TryGet(ComponentId p_id) {
    const size_t idx = (size_t)p_id;
    if (idx >= m_entries.size()) return nullptr;
    return m_entries[idx].pool.get();
}

const IComponentPool* ComponentStorage::TryGet(ComponentId p_id) const {
    const size_t idx = (size_t)p_id;
    if (idx >= m_entries.size()) return nullptr;
    return m_entries[idx].pool.get();
}

bool ComponentStorage::Has(Entity p_ent, ComponentId p_id) const {
    auto* pool = TryGet(p_id);
    return pool ? pool->Has(p_ent) : false;
}

void* ComponentStorage::GetRaw(Entity p_ent, ComponentId p_id) {
    auto* pool = TryGet(p_id);
    return pool ? pool->GetRaw(p_ent) : nullptr;
}

const void* ComponentStorage::GetRaw(Entity p_ent, ComponentId p_id) const {
    auto* pool = TryGet(p_id);
    return pool ? pool->GetRaw(p_ent) : nullptr;
}

void* ComponentStorage::CreateRaw(Entity p_ent, ComponentId p_id) {
    auto& pool = GetOrCreate(p_id);
    return pool.CreateRaw(p_ent);
}

bool ComponentStorage::Remove(Entity p_ent, ComponentId p_id) {
    if (!p_ent.IsValid()) return false;
    auto* pool = TryGet(p_id);
    if (!pool) return false;
    if (!pool->Has(p_ent)) return true;
    pool->Remove(p_ent);
    return true;
}

void ComponentStorage::Ensure(ComponentId p_id) {
    const size_t need = p_id + 1;
    if (m_entries.size() < need) m_entries.resize(need);
}

}  // namespace cave::ecs
