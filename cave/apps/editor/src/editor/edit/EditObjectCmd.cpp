#include "EditObjectCmd.h"

#include "cave/runtime/scene/SceneCommandPlayback.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

#include "editor/Enums.h"

namespace cave {

[[maybe_unused]] static std::string GenerateName(std::string_view name) {
    static int s_counter = 0;
    return std::format("{}-{}", name, ++s_counter);
}

bool DeleteObjectCmd::apply(IDocument& doc) {
    if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&doc)) {
        if (Scene* scene = resolveScene(scene_doc->previewScene())) {
            scene->removeEntity(m_ent);
            return true;
        }
    }
    return false;
}

bool DeleteObjectCmd::undo(IDocument&) {
    LOG_WARN("TODO: implement DeleteObjectCmd::Undo");
    return false;
}

bool CloneObjectCmd::apply(IDocument& doc) {
    if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&doc)) {
        if (Scene* scene = resolveScene(scene_doc->previewScene())) {
            scene->duplicateEntity(m_ent);
            return true;
        }
    }
    return true;
}

bool CloneObjectCmd::undo(IDocument&) {
    LOG_WARN("TODO: implement CloneObjectCmd::Undo");
    return false;
}

}  // namespace cave
