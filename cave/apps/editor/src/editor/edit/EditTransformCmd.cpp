#include "EditTransformCmd.h"

#include "cave/runtime/ecs/components/TransformComponent.h"

#include "editor/document/IDocument.h"

namespace cave {

bool EditTransformCmd::Do(IDocument& p_doc) {
    if (SceneId scene_id = p_doc.GetPreviewScene(); scene_id.IsValid()) {
        if (Scene* scene = ResolveScene(scene_id)) {
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
    if (SceneId scene_id = p_doc.GetPreviewScene(); scene_id.IsValid()) {
        if (Scene* scene = ResolveScene(scene_id)) {
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
