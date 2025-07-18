#include "editor_window.h"

namespace cave {

void EditorWindow::Update(Scene* scene) {
    if (ImGui::Begin(GetTitle(), nullptr, m_flags)) {
        m_is_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
        m_is_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
        UpdateInternal(scene);
    } else {
        m_is_focused = false;
        m_is_hovered = false;
    }
    ImGui::End();
}

}  // namespace cave
