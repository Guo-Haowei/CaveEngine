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

ChangePropertyCmd::ChangePropertyCmd(SceneRegistry& scene_reg,
                                     ecs::Entity ent,
                                     ComponentId cid,
                                     const PropertyId& pid,
                                     const void* old_data,
                                     const void* new_data,
                                     uint32_t data_size)
    : EditCmdBase(scene_reg, ent)
    , cid_(cid)
    , pid_(pid) {
    old_.resize(data_size);
    new_.resize(data_size);

    const void* old = old_data ? old_data : new_data;
    std::memcpy(old_.data(), old, data_size);
    std::memcpy(new_.data(), new_data, data_size);
}

bool ChangePropertyCmd::apply(IDocument& doc) {
    SceneId scene_id = doc.previewScene();
    if (!scene_id.valid()) return false;
    Scene* scene = resolveScene(scene_id);
    if (!scene) return false;

    SceneCommandExecutor executor(*scene);
    bool res = executor.changeProperty(m_ent,
                                       cid_,
                                       pid_,
                                       new_.data(),
                                       (uint32_t)new_.size());
    DEBUG_PRINT("Do: changed '{}' of entity {}", pid_.debugName(), m_ent.GetId());
    return res;
}

bool ChangePropertyCmd::undo(IDocument& doc) {
    SceneId scene_id = doc.previewScene();
    if (!scene_id.valid()) return false;
    Scene* scene = resolveScene(scene_id);
    if (!scene) return false;

    SceneCommandExecutor executor(*scene);
    bool res = executor.changeProperty(m_ent,
                                       cid_,
                                       pid_,
                                       old_.data(),
                                       (uint32_t)old_.size());
    DEBUG_PRINT("Undo: changed '{}' of entity {}", pid_.debugName(), m_ent.GetId());
    return res;
}

bool ChangePropertyCmd::canCoalesceWith(const IEditCmd* edit_cmd) const {
    if (const Self* cmd = dynamic_cast<const Self*>(edit_cmd)) {
        return cmd->m_ent == cmd->m_ent &&
               cid_ == cmd->cid_ &&
               pid_ == cmd->pid_;
    }
    return false;
}

void ChangePropertyCmd::coalesceFrom(std::unique_ptr<IEditCmd> edit_cmd) {
    Self& cmd = dynamic_cast<Self&>(*edit_cmd);
    new_ = std::move(cmd.new_);
}

}  // namespace cave
