#include "EditTransformCmd.h"

#include "editor/document/SceneDocument.h"

namespace cave {

bool EditTransformCmd::Do(IDocument& p_doc) {
    if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&p_doc)) {
        if (Scene* scene = ResolveScene(scene_doc->GetPreviewScene())) {
            TransformComponent* transform = scene->GetComponent<TransformComponent>(m_entity);
            if (transform) {
                transform->SetLocalTransform(m_after);
                return true;
            }
        }
    }

    return false;
}

bool EditTransformCmd::Undo(IDocument& p_doc) {
    if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&p_doc)) {
        if (Scene* scene = ResolveScene(scene_doc->GetPreviewScene())) {
            TransformComponent* transform = scene->GetComponent<TransformComponent>(m_entity);
            if (transform) {
                transform->SetLocalTransform(m_before);
                return true;
            }
        }
    }
    return false;
}

bool EditTransformCmd::CanCoalesceWith(const IEditCmd* p_cmd) const {
    if (const EditTransformCmd* cmd = dynamic_cast<const EditTransformCmd*>(p_cmd)) {
        if (m_entity == cmd->m_entity) {
            return true;
        }
    }

    return false;
}

void EditTransformCmd::CoalesceFrom(std::unique_ptr<IEditCmd> p_cmd) {
    EditTransformCmd& cmd = reinterpret_cast<EditTransformCmd&>(*p_cmd);
    m_after = cmd.m_after;
}

}  // namespace cave
