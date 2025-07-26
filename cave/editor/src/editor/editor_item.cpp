#include "editor_item.h"

#include "editor/editor_layer.h"

namespace cave {

void EditorItem::OpenAddEntityPopup(ecs::Entity p_parent) {
    if (ImGui::BeginMenu("Add")) {
#define ENTITY_TYPE(NAME, SEP)                                 \
    if (ImGui::MenuItem(#NAME)) {                              \
        m_editor.CommandAddEntity(EntityType::NAME, p_parent); \
    }                                                          \
    if constexpr (SEP) {                                       \
        ImGui::Separator();                                    \
    }
        ENTITY_TYPE_LIST
#undef ENTITY_TYPE
        ImGui::EndMenu();
    }
}

}  // namespace cave
