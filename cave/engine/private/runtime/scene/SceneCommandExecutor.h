#pragma once
#include "cave/runtime/ecs/ComponentRegistry.h"
#include "cave/runtime/scene/ISceneCommandExecutor.h"

namespace cave {

class Scene;

class SceneCommandExecutor : public ISceneCommandExecutor {
public:
    explicit SceneCommandExecutor(Scene& scene, ecs::ComponentRegistry& reg) noexcept;
    explicit SceneCommandExecutor(Scene& scene) noexcept;

    void addComponent(ecs::Entity ent, ComponentId id) override;

    bool removeComponent(ecs::Entity ent, ComponentId id) override;

    bool changeProperty(ecs::Entity ent,
                        ComponentId cid,
                        const PropertyId& pid,
                        const void* new_value,
                        uint32_t data_size) override;

protected:
    Scene& m_scene;
    const ecs::ComponentRegistry& m_reg;
};

}  // namespace cave
