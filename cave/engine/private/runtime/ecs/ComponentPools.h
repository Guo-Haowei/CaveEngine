#pragma once
#include "ComponentManager.h"
#include "ComponentManager.inl"

namespace cave::ecs {

using ComponentId = uint32_t;

class ComponentPools {
public:
    template<ComponentType T>
    ecs::ComponentManager<T>& Register(ComponentId id, const char* name, uint64_t version = 0);

    ecs::IComponentManager* TryGet(ComponentId id);
    const ecs::IComponentManager* TryGet(ComponentId id) const;

private:
    struct Entry {
        std::unique_ptr<ecs::IComponentManager> mgr;
        const char* name = nullptr;
        uint64_t version = 0;
    };
    std::vector<Entry> m_entries;
};

}  // namespace cave::ecs
