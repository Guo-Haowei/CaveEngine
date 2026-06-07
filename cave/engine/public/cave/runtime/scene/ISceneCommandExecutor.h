// =============================================================================
// File: cave/runtime/scene/ISceneCommandExecutor.h
// =============================================================================
#pragma once
#include "cave/runtime/ecs/ComponentDefines.h"
#include "cave/runtime/ecs/Entity.h"

namespace cave {

class ISceneCommandExecutor {
public:
    virtual ~ISceneCommandExecutor() = default;

    virtual void AddComponent(ecs::Entity p_ent, ComponentId p_cid) = 0;

    virtual bool RemoveComponent(ecs::Entity p_ent, ComponentId p_cid) = 0;

    virtual bool ChangeProperty(ecs::Entity p_ent,
                                ComponentId p_cid,
                                const PropertyId& p_pid,
                                const void* p_data,
                                uint32_t p_data_size) = 0;
};

}  // namespace cave
