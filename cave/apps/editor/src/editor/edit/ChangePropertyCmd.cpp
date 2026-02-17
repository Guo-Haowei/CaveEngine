#pragma once
#include "ChangePropertyCmd.h"

#include "engine/private/runtime/scene/SceneCommandExecutor.h"
#include "editor/document/IDocument.h"

namespace cave {

// #define DEBUG_EDIT_PROPERTY USE_IF(USING(USE_LOG) && USING(STRING_ID_KEEKP_SOURCE))
#define DEBUG_EDIT_PROPERTY NOT_IN_USE
#if USING(DEBUG_EDIT_PROPERTY)
#define DEBUG_PRINT(FMT, ...) LOG_VERBOSE("ChangePropertyCmd::" FMT, __VA_ARGS__)
#else
#define DEBUG_PRINT(...) (void)0
#endif

ChangePropertyCmd::ChangePropertyCmd(SceneRegistry& p_scene_reg,
                                     ecs::Entity p_ent,
                                     ComponentId p_cid,
                                     const PropertyId& p_pid,
                                     const void* p_old_data,
                                     const void* p_new_data,
                                     uint32_t p_data_size)
    : EditCmdBase(p_scene_reg, p_ent)
    , m_cid(p_cid)
    , m_pid(p_pid) {
    m_old.resize(p_data_size);
    m_new.resize(p_data_size);

    const void* old = p_old_data ? p_old_data : p_new_data;
    std::memcpy(m_old.data(), old, p_data_size);
    std::memcpy(m_new.data(), p_new_data, p_data_size);
}

bool ChangePropertyCmd::Do(IDocument& p_doc) {
    SceneId scene_id = p_doc.GetPreviewScene();
    if (!scene_id.IsValid()) return false;
    Scene* scene = ResolveScene(scene_id);
    if (!scene) return false;

    SceneCommandExecutor executor(*scene);
    bool res = executor.ChangeProperty(m_ent,
                                       m_cid,
                                       m_pid,
                                       m_new.data(),
                                       (uint32_t)m_new.size());
    DEBUG_PRINT("Do: changed '{}' of entity {}", m_prop_id.Source(), m_ent.GetId());
    return res;
}

bool ChangePropertyCmd::Undo(IDocument& p_doc) {
    SceneId scene_id = p_doc.GetPreviewScene();
    if (!scene_id.IsValid()) return false;
    Scene* scene = ResolveScene(scene_id);
    if (!scene) return false;

    SceneCommandExecutor executor(*scene);
    bool res = executor.ChangeProperty(m_ent,
                                       m_cid,
                                       m_pid,
                                       m_old.data(),
                                       (uint32_t)m_old.size());
    DEBUG_PRINT("Undo: changed '{}' of entity {}", m_prop_id.Source(), m_ent.GetId());
    return res;
}

bool ChangePropertyCmd::CanCoalesceWith(const IEditCmd* p_cmd) const {
    if (const Self* cmd = dynamic_cast<const Self*>(p_cmd)) {
        return cmd->m_ent == cmd->m_ent &&
               m_cid == cmd->m_cid &&
               m_pid == cmd->m_pid;
    }
    return false;
}

void ChangePropertyCmd::CoalesceFrom(std::unique_ptr<IEditCmd> p_cmd) {
    Self& cmd = dynamic_cast<Self&>(*p_cmd);
    m_new = std::move(cmd.m_new);
}

}  // namespace cave
