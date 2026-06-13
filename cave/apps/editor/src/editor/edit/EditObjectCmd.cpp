#include "EditObjectCmd.h"

#include "cave/runtime/scene/SceneCommandPlayback.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

#include "editor/Enums.h"

namespace cave {

[[maybe_unused]] static std::string GenerateName(std::string_view p_name) {
    static int s_counter = 0;
    return std::format("{}-{}", p_name, ++s_counter);
}

bool DeleteObjectCmd::Do(IDocument& p_doc) {
    if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&p_doc)) {
        if (Scene* scene = ResolveScene(scene_doc->previewScene())) {
            scene->RemoveEntity(m_ent);
            return true;
        }
    }
    return false;
}

bool DeleteObjectCmd::Undo(IDocument&) {
    LOG_WARN("TODO: implement DeleteObjectCmd::Undo");
    return false;
}

bool CloneObjectCmd::Do(IDocument& p_doc) {
    if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&p_doc)) {
        if (Scene* scene = ResolveScene(scene_doc->previewScene())) {
            scene->DuplicateEntity(m_ent);
            return true;
        }
    }
    return true;
}

bool CloneObjectCmd::Undo(IDocument&) {
    LOG_WARN("TODO: implement CloneObjectCmd::Undo");
    return false;
}

}  // namespace cave
