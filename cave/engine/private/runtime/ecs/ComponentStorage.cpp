#include "cave/runtime/ecs/ComponentStorage.h"

#include "ComponentPool.h"

// @TODO: refactor
// #include "engine/private/runtime/framework/Engine.h"
#include "engine/private/runtime/ecs/components/All.h"

namespace cave::ecs {

void ComponentStorage::clearAll() {
    for (auto& entry : entries_) {
        if (entry.pool) {
            entry.pool->clear();
        }
    }
}

IComponentPool& ComponentStorage::getOrCreate(ComponentId cid) {
    ensure(cid);
    Entry& e = entries_[cid];

    // @TODO: instead of this, make pool typeless
    if (!e.pool) {
        switch (cid) {
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

bool ComponentStorage::isRegistered(ComponentId cid) const {
    const size_t idx = cid;
    return idx < entries_.size() && entries_[idx].pool != nullptr;
}

IComponentPool* ComponentStorage::tryGet(ComponentId cid) {
    const size_t idx = (size_t)cid;
    if (idx >= entries_.size()) return nullptr;
    return entries_[idx].pool.get();
}

const IComponentPool* ComponentStorage::tryGet(ComponentId cid) const {
    const size_t idx = (size_t)cid;
    if (idx >= entries_.size()) return nullptr;
    return entries_[idx].pool.get();
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
    if (!ent.IsValid()) return false;
    auto* pool = tryGet(cid);
    if (!pool) return false;
    if (!pool->has(ent)) return true;
    pool->remove(ent);
    return true;
}

void ComponentStorage::ensure(ComponentId cid) {
    const size_t need = cid + 1;
    if (entries_.size() < need) entries_.resize(need);
}

}  // namespace cave::ecs
