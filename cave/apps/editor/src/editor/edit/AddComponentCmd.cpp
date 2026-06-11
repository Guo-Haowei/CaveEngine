#include "AddComponentCmd.h"

#include "engine/private/runtime/scene/SceneCommandExecutor.h"
#include "editor/document/SceneDocument.h"

namespace cave {

AddComponentCmd::AddComponentCmd(SceneRegistry& p_scene_reg,
                                 ecs::Entity p_ent,
                                 ComponentId p_cid)
    : EditCmdBase(p_scene_reg, p_ent)
    , m_cid(p_cid) {
}

bool AddComponentCmd::Do(IDocument& p_doc) {
    if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&p_doc)) {
        if (Scene* scene = ResolveScene(scene_doc->previewScene())) {
            SceneCommandExecutor executor(*scene);
            executor.AddComponent(m_ent, m_cid);
            return true;
        }
    }
    return false;
}

bool AddComponentCmd::Undo(IDocument& p_doc) {
    if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&p_doc)) {
        if (Scene* scene = ResolveScene(scene_doc->previewScene())) {
            SceneCommandExecutor executor(*scene);
            executor.RemoveComponent(m_ent, m_cid);
            return true;
        }
    }
    return false;
}

}  // namespace cave
