#pragma once
#include "cave/runtime/ecs/ComponentRegistry.h"
#include "cave/runtime/scene/ISceneCommandExecutor.h"

namespace cave {

class Scene;

class SceneCommandExecutor : public ISceneCommandExecutor {
public:
    explicit SceneCommandExecutor(Scene& p_scene, ecs::ComponentRegistry& p_reg) noexcept;
    explicit SceneCommandExecutor(Scene& p_scene) noexcept;

    void addComponent(ecs::Entity p_ent, ComponentId p_id) override;

    bool removeComponent(ecs::Entity p_ent, ComponentId p_id) override;

    bool changeProperty(ecs::Entity p_ent,
                        ComponentId p_cid,
                        const PropertyId& p_pid,
                        const void* p_data,
                        uint32_t p_data_size) override;

protected:
    Scene& m_scene;
    const ecs::ComponentRegistry& m_reg;
};

}  // namespace cave
