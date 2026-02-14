// =============================================================================
// File: engine/public/cave/runtime/ecs/ComponentStorage.h
// =============================================================================
#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/runtime/ecs/ComponentDefines.h"
#include "cave/runtime/ecs/ComponentRegistry.h"

namespace cave::ecs {

class IComponentPool;

template<ComponentType T>
class ComponentPool;

class ComponentStorage {
public:
    ComponentStorage() = default;

    void ClearAll();

    bool IsRegistered(ComponentId p_id) const;

    IComponentPool* TryGet(ComponentId p_id) const;

    IComponentPool* FindByName(const StringId& p_id) const;

    // Typed registration: keeps your typed vector storage
    template<typename ManagerT>
    ManagerT& Register(ComponentId p_id) {
        Ensure(p_id);
        Entry& e = m_entries[p_id];
        DEV_ASSERT(e.pool == nullptr);
        e.pool = std::make_unique<ManagerT>();
        return static_cast<ManagerT&>(*e.pool);
    }

    template<ComponentType T>
    ComponentPool<T>& RegisterTyped(ComponentId p_id, size_t p_reserve = 0) {
        auto& pool = Register<ComponentPool<T>>(p_id);
        if (p_reserve) {
            pool.Reserve(p_reserve);
        }
        return pool;
    }

    bool Has(Entity p_ent, ComponentId p_id) const;

    void* GetRaw(Entity p_ent, ComponentId p_id);

    void* AddDefault(Entity p_ent, ComponentId p_id);

    bool Remove(Entity p_ent, ComponentId p_id);

private:
    struct Entry {
        std::unique_ptr<IComponentPool> pool;
    };

    void Ensure(ComponentId p_id);

    std::vector<Entry> m_entries;
};

}  // namespace cave::ecs
