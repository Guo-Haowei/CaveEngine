// =============================================================================
// File: cave/runtime/ecs/ComponentStorage.h
// =============================================================================
#pragma once
#include "cave/core/containers/Containers.h"
#include "cave/runtime/ecs/ComponentRegistry.h"
#include "cave/core/ids/Entity.h"

// clang-format off
namespace cave { class Scene; }
// clang-format on

namespace cave::ecs {

class IComponentPool;

template<ComponentType T>
class ComponentPool;

class ComponentStorage {
    struct Entry {
        Owner<IComponentPool> pool;
    };

public:
    explicit ComponentStorage() = default;

    void clearAll();

    bool isRegistered(ComponentId cid) const;

    IComponentPool& getOrCreate(ComponentId cid);

    template<ComponentType T>
    IComponentPool& getOrCreate() {
        return getOrCreate(T::kId);
    }

    IComponentPool* tryGet(ComponentId cid);

    const IComponentPool* tryGet(ComponentId cid) const;

    bool has(ComponentId cid, Entity ent) const;

    void* getRaw(ComponentId cid, Entity ent);

    const void* getRaw(ComponentId cid, Entity ent) const;

    void* createRaw(ComponentId cid, Entity ent);

    bool remove(ComponentId cid, Entity ent);

    std::span<const Entry> entries() const { return m_entries; }

private:
    ComponentStorage(ComponentStorage&) = delete;
    ComponentStorage& operator=(ComponentStorage&) = delete;

    void ensure(ComponentId cid);

    Vector<Entry> m_entries;

    friend class ::cave::Scene;
};

}  // namespace cave::ecs
