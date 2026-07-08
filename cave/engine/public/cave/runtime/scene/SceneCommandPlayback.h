// =============================================================================
// File: cave/runtime/scene/SceneCommandPlayback.h
// =============================================================================
#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/runtime/scene/SceneCommandBuffer.h"
#include "cave/runtime/scene/ISceneCommandExecutor.h"

namespace cave {

class Scene;

class EntityMap {
public:
    explicit EntityMap(uint32_t reserve);

    void setRemap(ecs::Entity temp, ecs::Entity real);

    ecs::Entity resolve(ecs::Entity ent) const noexcept;

private:
    Vector<ecs::Entity> m_remap;
};

struct SceneCommandPlayback {
    struct Context {
        EntityMap& map;
        Scene& scene;
    };

    static void Play(SceneCommandBuffer& cb,
                     ISceneCommandExecutor& exe,
                     const Context& ctx);
};

}  // namespace cave
