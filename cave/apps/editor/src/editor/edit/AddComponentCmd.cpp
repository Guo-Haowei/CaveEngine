#include "AddComponentCmd.h"

#include "cave/runtime/scene/SceneCommandExecutor.h"
#include "editor/document/SceneDocument.h"

namespace cave {

AddComponentCmd::AddComponentCmd(IApplication& p_app,
                                 ecs::Entity p_ent,
                                 BuiltinComponentId p_cid)
    : EditCmdBase(p_app, p_ent)
    , m_cid(p_cid) {
}

bool AddComponentCmd::Do(IDocument& p_doc) {
    if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&p_doc)) {
        if (Scene* scene = ResolveScene(scene_doc->GetPreviewScene())) {
            SceneCommandExecutor executor(*scene);
            executor.AddComponent(m_ent, m_cid);
            return true;
        }
    }
    return false;
}

bool AddComponentCmd::Undo(IDocument& p_doc) {
    if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&p_doc)) {
        if (Scene* scene = ResolveScene(scene_doc->GetPreviewScene())) {
            SceneCommandExecutor executor(*scene);
            executor.RemoveComponent(m_ent, m_cid);
            return true;
        }
    }
    return false;
}

}  // namespace cave
