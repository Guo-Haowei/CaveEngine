// =============================================================================
// File: cave/runtime/ecs/IComponentPool.h
// =============================================================================
#pragma once
#include <span>

#include "cave/core/containers/Containers.h"
#include "cave/core/ids/Entity.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave::ecs {

class IComponentPool {
    IComponentPool(const IComponentPool&) = delete;
    IComponentPool& operator=(const IComponentPool&) = delete;

public:
    IComponentPool() = default;
    virtual ~IComponentPool() = default;
    virtual void clear() = 0;
    virtual void copy(const IComponentPool& other) = 0;
    virtual Owner<IComponentPool> clone() const = 0;

    virtual void* createRaw(Entity ent) = 0;
    virtual void remove(Entity ent) = 0;

    bool has(Entity ent) const;

    virtual void* getRaw(Entity ent) = 0;
    virtual const void* getRaw(Entity ent) const = 0;

    virtual void merge(IComponentPool&& other) = 0;
    virtual size_t count() const = 0;

    void remap(const HashMap<Entity, Entity>& map);

    const Vector<Entity>& entityArray() const {
        return m_entity_array;
    }

protected:
    Vector<Entity> m_entity_array;
    HashMap<Entity, size_t> m_lookup;
};

}  // namespace cave::ecs
