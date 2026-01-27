#include "EditorItem.h"

#include "editor/EditorState.h"

#include "editor/services/EditService.h"
#include "editor/services/Workspace.h"

namespace cave {

void EditorItem::OpenAddEntityPopup(ecs::Entity p_parent) {
    ViewerTab* tab = m_editor.GetWorkspace().GetActiveTab();
    SceneId scene_id = tab ? tab->GetSceneId() : SceneId{};

    if (ImGui::BeginMenu("Add")) {
#define ENTITY_TYPE(NAME, SEP)                                          \
    if (ImGui::MenuItem(#NAME)) {                                       \
        m_editor.GetEditService().CommandCreateObject(scene_id,         \
                                                      EntityType::NAME, \
                                                      p_parent);        \
    }                                                                   \
    if constexpr (SEP) {                                                \
        ImGui::Separator();                                             \
    }
        ENTITY_TYPE_LIST
#undef ENTITY_TYPE
        ImGui::EndMenu();
    }
}

}  // namespace cave
