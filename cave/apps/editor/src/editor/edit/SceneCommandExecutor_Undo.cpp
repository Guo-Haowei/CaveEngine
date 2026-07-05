#include "SceneCommandExecutor_Undo.h"

#include "editor/edit/AddComponentCmd.h"
#include "editor/edit/ChangePropertyCmd.h"  // @TODO: rename to Edit
#include "editor/edit/CompositeEditCmd.h"

namespace cave {

using ecs::Entity;

SceneCommandExecutor_Undo::SceneCommandExecutor_Undo(SceneRegistry& p_scene_reg) noexcept
    : m_scene_reg(p_scene_reg) {
    m_cmd = std::make_unique<CompositeEditCmd>();
}

SceneCommandExecutor_Undo::~SceneCommandExecutor_Undo() = default;

void SceneCommandExecutor_Undo::addComponent(Entity p_ent, ComponentId p_cid) {
    auto cmd = std::make_unique<AddComponentCmd>(m_scene_reg,
                                                 p_ent,
                                                 p_cid);
    m_cmd->AddCommand(std::move(cmd));
}

bool SceneCommandExecutor_Undo::removeComponent(Entity p_ent, ComponentId p_cid) {
    unused(p_ent);
    unused(p_cid);
    CRASH_NOW_MSG("not implemented");
    return false;
}

bool SceneCommandExecutor_Undo::changeProperty(Entity p_ent,
                                               ComponentId p_cid,
                                               const PropertyId& p_pid,
                                               const void* p_data,
                                               uint32_t p_data_size) {
    auto cmd = std::make_unique<ChangePropertyCmd>(
        m_scene_reg,
        p_ent,
        p_cid,
        p_pid,
        nullptr,  // composite command, don't care about old value
        p_data,
        p_data_size);

    m_cmd->AddCommand(std::move(cmd));
    return true;
}

std::unique_ptr<IEditCmd> SceneCommandExecutor_Undo::MoveCommand() {
    return std::move(m_cmd);
}

}  // namespace cave