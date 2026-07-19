#include "ComponentPool.h"

#include "cave/runtime/ecs/ComponentStorage.h"
#include "cave/runtime/ui/UIComponents.h"

// @TODO: refactor
// #include "engine/private/runtime/framework/Engine.h"
#include "engine/private/runtime/ecs/components/All.h"

namespace cave::ecs {

void ComponentStorage::clearAll() {
    for (auto& entry : m_entries) {
        if (entry.pool) {
            entry.pool->clear();
        }
    }
}

IComponentPool& ComponentStorage::getOrCreate(ComponentId cid) {
    const uint32_t idx = ensure(cid);
    Entry& e = m_entries[idx];
    DEV_ASSERT(e.type_id == cid);

    // @TODO: make pool typeless
    if (!e.pool) {
        switch (cid.hash()) {
#define REGISTER_COMPONENT(TYPE, ...)              \
    case TYPE##_hash: {                            \
        e.pool = MakeOwner<ComponentPool<TYPE>>(); \
    } break;
            REGISTER_COMPONENT_LIST
#undef REGISTER_COMPONENT
            default:
                CRASH_NOW_MSG("Unkown component type");
                break;
        }
    }

    return *e.pool;
}

bool ComponentStorage::isRegistered(ComponentId cid) const {
    auto it = m_lookup.find(cid);
    if (it == m_lookup.end()) return false;

    const size_t idx = it->second;
    return idx < m_entries.size() && m_entries[idx].pool != nullptr;
}

IComponentPool* ComponentStorage::tryGet(ComponentId cid) {
    auto it = m_lookup.find(cid);
    if (it == m_lookup.end()) return nullptr;

    const size_t idx = it->second;
    if (DEV_VERIFY(idx < m_entries.size())) {
        return m_entries[idx].pool.get();
    }
    return nullptr;
}

const IComponentPool* ComponentStorage::tryGet(ComponentId cid) const {
    auto it = m_lookup.find(cid);
    if (it == m_lookup.end()) return nullptr;

    const size_t idx = it->second;
    if (DEV_VERIFY(idx < m_entries.size())) {
        return m_entries[idx].pool.get();
    }
    return nullptr;
}

bool ComponentStorage::has(ComponentId cid, Entity ent) const {
    auto* pool = tryGet(cid);
    return pool ? pool->has(ent) : false;
}

void* ComponentStorage::getRaw(ComponentId cid, Entity ent) {
    auto* pool = tryGet(cid);
    return pool ? pool->getRaw(ent) : nullptr;
}

const void* ComponentStorage::getRaw(ComponentId cid, Entity ent) const {
    auto* pool = tryGet(cid);
    return pool ? pool->getRaw(ent) : nullptr;
}

void* ComponentStorage::createRaw(ComponentId cid, Entity ent) {
    auto& pool = getOrCreate(cid);
    return pool.createRaw(ent);
}

bool ComponentStorage::remove(ComponentId cid, Entity ent) {
    if (!ent.valid()) return false;
    auto* pool = tryGet(cid);
    if (!pool) return false;
    if (!pool->has(ent)) return true;
    pool->remove(ent);
    return true;
}

uint32_t ComponentStorage::ensure(ComponentId cid) {
    const uint32_t size = static_cast<uint32_t>(m_entries.size());
    auto [it, inserted] = m_lookup.try_emplace(cid, size);

    if (inserted) {
        m_entries.emplace_back(cid);
    }
    return it->second;
}

}  // namespace cave::ecs
