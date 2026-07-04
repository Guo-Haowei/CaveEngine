// =============================================================================
// File: cave/runtime/ecs/ComponentStorage.h
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

    std::span<const Entry> entries() const { return entries_; }

private:
    ComponentStorage(ComponentStorage&) = delete;
    ComponentStorage& operator=(ComponentStorage&) = delete;

    void ensure(ComponentId cid);

    std::vector<Entry> entries_;

    friend class ::cave::Scene;
};

}  // namespace cave::ecs
