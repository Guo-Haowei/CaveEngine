#pragma once
#include "cave/runtime/scene/ISceneCommandExecutor.h"

namespace cave {

class CompositeEditCmd;
class IEditCmd;
class IApplication;
class SceneRegistry;

class SceneCommandExecutor_Undo final : public ISceneCommandExecutor {
public:
    explicit SceneCommandExecutor_Undo(SceneRegistry& p_scene_reg) noexcept;

    ~SceneCommandExecutor_Undo();

    void RemoveEntity(ecs::Entity p_ent) override;

    void AddComponent(ecs::Entity p_ent, ComponentId p_cid) override;

    bool RemoveComponent(ecs::Entity p_ent, ComponentId p_cid) override;

    bool ChangeProperty(ecs::Entity p_ent,
                        ComponentId p_cid,
                        const PropertyId& p_pid,
                        const void* p_data,
                        uint32_t p_data_size) override;

    std::unique_ptr<IEditCmd> MoveCommand();

private:
    SceneRegistry& m_scene_reg;

    std::unique_ptr<CompositeEditCmd> m_cmd;
};

}  // namespace cave
