#pragma once
#include "EditPropertyCmd.h"

#include "cave/runtime/scene/SceneMutator.h"
#include "editor/document/IDocument.h"

namespace cave {

bool EditPropertyCmd::Do(IDocument& p_doc) {
    SceneId scene_id = p_doc.GetPreviewScene();
    if (!scene_id.IsValid()) return false;
    Scene* scene = ResolveScene(scene_id);
    if (!scene) return false;

    SceneMutator mut(*scene);
    bool res = mut.ChangeProperty(m_entity,
                                  m_id,
                                  m_property,
                                  m_new.data(),
                                  (uint32_t)m_new.size());
    return res;
}

bool EditPropertyCmd::Undo(IDocument& p_doc) {
    SceneId scene_id = p_doc.GetPreviewScene();
    if (!scene_id.IsValid()) return false;
    Scene* scene = ResolveScene(scene_id);
    if (!scene) return false;

    SceneMutator mut(*scene);
    bool res = mut.ChangeProperty(m_entity,
                                  m_id,
                                  m_property,
                                  m_old.data(),
                                  (uint32_t)m_old.size());
    return res;
}

bool EditPropertyCmd::CanCoalesceWith(const IEditCmd* p_cmd) const {
    if (const Self* cmd = dynamic_cast<const Self*>(p_cmd)) {
        return cmd->m_entity == cmd->m_entity &&
               m_property == cmd->m_property;
    }
    return false;
}

void EditPropertyCmd::CoalesceFrom(std::unique_ptr<IEditCmd> p_cmd) {
    Self& cmd = dynamic_cast<Self&>(*p_cmd);
    m_new = std::move(cmd.m_new);
}

}  // namespace cave
