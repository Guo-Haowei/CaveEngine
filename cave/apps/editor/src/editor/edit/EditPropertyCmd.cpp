#pragma once
#include "EditPropertyCmd.h"

#include "cave/runtime/scene/SceneCommandExecutor.h"
#include "editor/document/IDocument.h"

namespace cave {

//#define DEBUG_EDIT_PROPERTY USE_IF(USING(USE_LOG) && USING(STRING_ID_KEEKP_SOURCE))
#define DEBUG_EDIT_PROPERTY NOT_IN_USE
#if USING(DEBUG_EDIT_PROPERTY)
#define DEBUG_PRINT(FMT, ...) LOG_VERBOSE("EditPropertyCmd::" FMT, __VA_ARGS__)
#else
#define DEBUG_PRINT(...) (void)0
#endif

bool EditPropertyCmd::Do(IDocument& p_doc) {
    SceneId scene_id = p_doc.GetPreviewScene();
    if (!scene_id.IsValid()) return false;
    Scene* scene = ResolveScene(scene_id);
    if (!scene) return false;

    SceneCommandExecutor mut(*scene);
    bool res = mut.ChangeProperty(m_entity,
                                  m_id,
                                  m_prop_id,
                                  m_new.data(),
                                  (uint32_t)m_new.size());
    DEBUG_PRINT("Do: changed '{}' of entity {}", m_prop_id.Source(), m_entity.GetId());
    return res;
}

bool EditPropertyCmd::Undo(IDocument& p_doc) {
    SceneId scene_id = p_doc.GetPreviewScene();
    if (!scene_id.IsValid()) return false;
    Scene* scene = ResolveScene(scene_id);
    if (!scene) return false;

    SceneCommandExecutor mut(*scene);
    bool res = mut.ChangeProperty(m_entity,
                                  m_id,
                                  m_prop_id,
                                  m_old.data(),
                                  (uint32_t)m_old.size());
    DEBUG_PRINT("Undo: changed '{}' of entity {}", m_prop_id.Source(), m_entity.GetId());
    return res;
}

bool EditPropertyCmd::CanCoalesceWith(const IEditCmd* p_cmd) const {
    if (const Self* cmd = dynamic_cast<const Self*>(p_cmd)) {
        return cmd->m_entity == cmd->m_entity &&
               m_prop_id == cmd->m_prop_id;
    }
    return false;
}

void EditPropertyCmd::CoalesceFrom(std::unique_ptr<IEditCmd> p_cmd) {
    Self& cmd = dynamic_cast<Self&>(*p_cmd);
    m_new = std::move(cmd.m_new);
}

}  // namespace cave
