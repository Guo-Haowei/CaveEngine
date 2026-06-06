// =============================================================================
// File: cave/runtime/ecs/IComponentPool.h
// =============================================================================
#pragma once
#include <vector>
#include <unordered_map>
#include "cave/runtime/ecs/ComponentDefines.h"
#include "cave/runtime/ecs/Entity.h"

namespace cave::ecs {

class IComponentPool {
    IComponentPool(const IComponentPool&) = delete;
    IComponentPool& operator=(const IComponentPool&) = delete;

public:
    IComponentPool() = default;
    virtual ~IComponentPool() = default;
    virtual void Clear() = 0;
    virtual void Copy(const IComponentPool& p_other) = 0;
    virtual std::unique_ptr<IComponentPool> Clone() const = 0;

    virtual void* CreateRaw(Entity p_ent) = 0;
    virtual void Remove(Entity p_ent) = 0;

    bool Has(Entity p_ent) const;

    virtual void* GetRaw(Entity p_ent) = 0;
    virtual const void* GetRaw(Entity p_ent) const = 0;

    virtual void Merge(IComponentPool&& p_other) = 0;
    virtual size_t GetCount() const = 0;

    void Remap(const std::unordered_map<Entity, Entity>& p_map);

    const std::vector<Entity>& GetEntityArray() const {
        return m_entity_array;
    }

protected:
    std::vector<Entity> m_entity_array;
    std::unordered_map<Entity, size_t> m_lookup;
};

}  // namespace cave::ecs
