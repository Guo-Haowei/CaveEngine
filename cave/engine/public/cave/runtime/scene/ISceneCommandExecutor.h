// =============================================================================
// File: cave/runtime/scene/ISceneCommandExecutor.h
// =============================================================================
#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

class ISceneCommandExecutor {
public:
    virtual ~ISceneCommandExecutor() = default;

    virtual void addComponent(ecs::Entity ent, ComponentId cid) = 0;
    virtual bool removeComponent(ecs::Entity ent, ComponentId cid) = 0;

    virtual bool changeProperty(ecs::Entity ent,
                                ComponentId cid,
                                const PropertyId& pid,
                                const void* data,
                                uint32_t data_size) = 0;
};

}  // namespace cave
