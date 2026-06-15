#include "AddComponentCmd.h"

#include "engine/private/runtime/scene/SceneCommandExecutor.h"
#include "editor/document/SceneDocument.h"

namespace cave {

AddComponentCmd::AddComponentCmd(SceneRegistry& scene_reg,
                                 ecs::Entity ent,
                                 ComponentId cid)
    : EditCmdBase(scene_reg, ent)
    , cid_(cid) {
}

bool AddComponentCmd::apply(IDocument& doc) {
    if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&doc)) {
        if (Scene* scene = resolveScene(scene_doc->previewScene())) {
            SceneCommandExecutor executor(*scene);
            executor.AddComponent(ent_, cid_);
            return true;
        }
    }
    return false;
}

bool AddComponentCmd::undo(IDocument& doc) {
    if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&doc)) {
        if (Scene* scene = resolveScene(scene_doc->previewScene())) {
            SceneCommandExecutor executor(*scene);
            executor.RemoveComponent(ent_, cid_);
            return true;
        }
    }
    return false;
}

}  // namespace cave
