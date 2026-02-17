#pragma once
#include "engine/private/runtime/scene/SceneCommandExecutor.h"

namespace cave {

class SceneRegistry;
class CompositeEditCmd;
class IApplication;

class SceneCommandExecutor_Undo final : public SceneCommandExecutor {
public:
    explicit SceneCommandExecutor_Undo(Scene& p_scene, SceneRegistry& p_scene_reg) noexcept;

    ~SceneCommandExecutor_Undo();

    void RemoveEntity(ecs::Entity p_ent) override;

    void AddComponent(ecs::Entity p_ent, ComponentId p_cid) override;

    bool RemoveComponent(ecs::Entity p_ent, ComponentId p_cid) override;

    bool ChangeProperty(ecs::Entity p_ent,
                        ComponentId p_cid,
                        const PropertyId& p_pid,
                        const void* p_data,
                        uint32_t p_data_size) override;

private:
    SceneRegistry& m_scene_reg;

    std::unique_ptr<CompositeEditCmd> m_cmd;
};

}  // namespace cave
