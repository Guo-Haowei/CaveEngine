#include "SceneDocument.h"

#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/ISceneRegistry.h"
#include "engine/private/runtime/scene/Scene.h"
#include "editor/undo_redo/UndoStack.h"

namespace cave {

SceneDocument::SceneDocument(IApplication& p_app, const Guid& p_guid)
    : DocumentBase(p_app, p_guid) {

    auto scene = std::make_unique<Scene>();
    scene->Copy(*m_handle.Get<Scene>());

    m_preview_scene = m_scene_reg.Register(std::move(scene));
}

bool SceneDocument::Save() {
    return false;
}

bool SceneDocument::SaveAs(std::string_view p_new_path) {
    unused(p_new_path);
    return false;
}

}  // namespace cave
