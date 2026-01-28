#include "EditorWindow.h"

namespace cave {

void EditorWindow::Update(float p_timestep) {
    m_state = {};

    if (ImGui::Begin(GetTitle(), nullptr, m_flags)) {
        m_state.open = true;
        m_state.visible = !ImGui::IsWindowCollapsed();
        m_state.focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
        m_state.hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
        UpdateInternal(p_timestep);
    }
    ImGui::End();
}

}  // namespace cave
