#include "editor_window.h"

namespace cave {

void EditorWindow::Update(float p_timestep) {
    if (ImGui::Begin(GetTitle(), nullptr, m_flags)) {
        m_is_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
        m_is_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
        UpdateInternal(p_timestep);
    } else {
        m_is_focused = false;
        m_is_hovered = false;
    }
    ImGui::End();
}

}  // namespace cave
