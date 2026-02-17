#include "SceneCommandExecutor_Undo.h"

#include "engine/private/runtime/scene/Scene.h"
#include "editor/edit/AddComponentCmd.h"
#include "editor/edit/ChangePropertyCmd.h"  // @TODO: rename to Edit
#include "editor/edit/CompositeEditCmd.h"

namespace cave {

using ecs::Entity;

SceneCommandExecutor_Undo::SceneCommandExecutor_Undo(Scene& p_scene, SceneRegistry& p_scene_reg) noexcept
    : SceneCommandExecutor(p_scene)
    , m_scene_reg(p_scene_reg) {
    m_cmd = std::make_unique<CompositeEditCmd>();
}

SceneCommandExecutor_Undo::~SceneCommandExecutor_Undo() = default;

void SceneCommandExecutor_Undo::RemoveEntity(Entity p_ent) {
    unused(p_ent);
    CRASH_NOW_MSG("not implemented");
}

void SceneCommandExecutor_Undo::AddComponent(Entity p_ent, ComponentId p_cid) {
    auto cmd = std::make_unique<AddComponentCmd>(m_scene_reg,
                                                 p_ent,
                                                 p_cid);
    m_cmd->AddCommand(std::move(cmd));
}

bool SceneCommandExecutor_Undo::RemoveComponent(Entity p_ent, ComponentId p_cid) {
    unused(p_ent);
    unused(p_cid);
    CRASH_NOW_MSG("not implemented");
    return false;
}

bool SceneCommandExecutor_Undo::ChangeProperty(Entity p_ent,
                                               ComponentId p_cid,
                                               const PropertyId& p_pid,
                                               const void* p_data,
                                               uint32_t p_data_size) {
    const ecs::ComponentMeta* meta = m_reg.TryGet(p_cid);
    if (!meta) {
        LOG_WARN("Can't find meta for component '{}'", p_cid);
        return false;
    }

    const void* old = ReadProperty(p_ent, meta, p_pid);
    if (DEV_VERIFY(old)) {
        auto cmd = std::make_unique<ChangePropertyCmd>(
            m_scene_reg,
            p_ent,
            p_cid,
            p_pid,
            old,
            p_data,
            p_data_size);

        m_cmd->AddCommand(std::move(cmd));
        return true;
    }

    return false;
}

}  // namespace cave