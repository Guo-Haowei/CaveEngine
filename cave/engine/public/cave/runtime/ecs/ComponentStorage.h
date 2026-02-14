// =============================================================================
// File: engine/public/cave/runtime/ecs/ComponentStorage.h
// =============================================================================
#pragma once
#include "cave/runtime/ecs/ComponentRegistry.h"
#include "cave/runtime/ecs/Entity.h"

namespace cave {
class Scene;
}

namespace cave::ecs {

class IComponentPool;

template<ComponentType T>
class ComponentPool;

class ComponentStorage {
    struct Entry {
        std::unique_ptr<IComponentPool> pool;
    };

public:
    explicit ComponentStorage() = default;

    void ClearAll();

    bool IsRegistered(ComponentId p_id) const;

    IComponentPool* TryGet(ComponentId p_id);

    const IComponentPool* TryGet(ComponentId p_id) const;

    template<ComponentType T>
    ComponentPool<T>& GetOrCreate() {
        constexpr ComponentId id = T::kId;
        Ensure(id);
        Entry& e = m_entries[id];
        if (e.pool == nullptr) {
            e.pool = std::make_unique<ComponentPool<T>>();
        }
        return static_cast<ComponentPool<T>&>(*e.pool);
    }

    bool Has(Entity p_ent, ComponentId p_id) const;

    void* GetRaw(Entity p_ent, ComponentId p_id);

    void* AddDefault(Entity p_ent, ComponentId p_id);

    bool Remove(Entity p_ent, ComponentId p_id);

    const auto& GetEntries() const { return m_entries; }

private:
    ComponentStorage(ComponentStorage&) = delete;
    ComponentStorage& operator=(ComponentStorage&) = delete;

    void Ensure(ComponentId p_id);

    std::vector<Entry> m_entries;

    friend class ::cave::Scene;
};

}  // namespace cave::ecs
