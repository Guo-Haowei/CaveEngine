// =============================================================================
// File: cave/runtime/ecs/ComponentStorage.h
// =============================================================================
#pragma once
#include "cave/core/containers/Containers.h"
#include "cave/core/ids/Entity.h"
#include "cave/core/string/StringId.h"
#include "cave/runtime/ecs/ComponentDefines.h"

// clang-format off
namespace cave { class Scene; }
// clang-format on

namespace cave::ecs {

class IComponentPool;

template<ComponentType T>
class ComponentPool;

class ComponentStorage {
    struct Entry {
        Entry(StringId type_id)
            : type_id(type_id) {}

        StringId type_id;
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

    uint32_t ensure(ComponentId cid);

    HashMap<ComponentId, uint32_t> m_lookup;
    Vector<Entry> m_entries;

    friend class ::cave::Scene;
};

}  // namespace cave::ecs
