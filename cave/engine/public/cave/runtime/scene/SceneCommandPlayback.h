// =============================================================================
// File: public/cave/runtime/scene/SceneCommandPlayback.h
// =============================================================================
#pragma once
#include <vector>
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/scene/SceneCommandBuffer.h"
#include "cave/runtime/scene/ISceneCommandExecutor.h"

namespace cave {

class EntityMap {
public:
    explicit EntityMap(uint32_t p_reserve);

    void SetRemap(ecs::Entity p_temp, ecs::Entity p_real);

    ecs::Entity Resolve(ecs::Entity p_ent) const noexcept;

private:
    std::vector<ecs::Entity> m_remap;
};

void SceneCommandPlayback(SceneCommandBuffer& p_cb,
                          ISceneCommandExecutor& p_exe,
                          EntityMap& p_map);

}  // namespace cave
