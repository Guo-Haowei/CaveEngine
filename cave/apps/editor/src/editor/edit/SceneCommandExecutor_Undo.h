#pragma once
#include "cave/runtime/scene/ISceneCommandExecutor.h"

namespace cave {

class CompositeEditCmd;
class IEditCmd;
class IApplication;
class SceneRegistry;

class SceneCommandExecutor_Undo final : public ISceneCommandExecutor {
public:
    explicit SceneCommandExecutor_Undo(SceneRegistry& scene_reg) noexcept;

    ~SceneCommandExecutor_Undo();

    void addComponent(ecs::Entity ent, ComponentId cid) override;

    bool removeComponent(ecs::Entity ent, ComponentId cid) override;

    bool changeProperty(ecs::Entity ent,
                        ComponentId cid,
                        const PropertyId& pid,
                        const void* data,
                        uint32_t data_size) override;

    std::unique_ptr<IEditCmd> MoveCommand();

private:
    SceneRegistry& m_scene_reg;

    std::unique_ptr<CompositeEditCmd> m_cmd;
};

}  // namespace cave
