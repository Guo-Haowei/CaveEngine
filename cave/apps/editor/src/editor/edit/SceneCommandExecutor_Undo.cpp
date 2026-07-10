#include "SceneCommandExecutor_Undo.h"

#include "editor/edit/AddComponentCmd.h"
#include "editor/edit/ChangePropertyCmd.h"  // @TODO: rename to Edit
#include "editor/edit/CompositeEditCmd.h"

namespace cave {

using ecs::Entity;

SceneCommandExecutor_Undo::SceneCommandExecutor_Undo(SceneRegistry& scene_reg) noexcept
    : m_scene_reg(scene_reg) {
    m_cmd = std::make_unique<CompositeEditCmd>();
}

SceneCommandExecutor_Undo::~SceneCommandExecutor_Undo() = default;

void SceneCommandExecutor_Undo::addComponent(Entity ent, ComponentId cid) {
    auto cmd = std::make_unique<AddComponentCmd>(m_scene_reg,
                                                 ent,
                                                 cid);
    m_cmd->AddCommand(std::move(cmd));
}

bool SceneCommandExecutor_Undo::removeComponent(Entity ent, ComponentId cid) {
    unused(ent);
    unused(cid);
    CRASH_NOW_MSG("not implemented");
    return false;
}

bool SceneCommandExecutor_Undo::changeProperty(Entity ent,
                                               ComponentId cid,
                                               const PropertyId& pid,
                                               const void* data,
                                               uint32_t data_size) {
    auto cmd = std::make_unique<ChangePropertyCmd>(
        m_scene_reg,
        ent,
        cid,
        pid,
        nullptr,  // composite command, don't care about old value
        data,
        data_size);

    m_cmd->AddCommand(std::move(cmd));
    return true;
}

Owner<IEditCmd> SceneCommandExecutor_Undo::takeCommand() {
    return std::move(m_cmd);
}

}  // namespace cave