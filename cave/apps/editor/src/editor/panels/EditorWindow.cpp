#include "EditorWindow.h"

namespace cave {

EditorWindow::EditorWindow(EditorState& p_editor)
    : IEditorItem(p_editor) {}

void EditorWindow::DrawUI() {
    ResetState();
    if (ImGui::Begin(GetWindowId(), nullptr, m_flags)) {
        UpdateState();
        DrawUIImpl();
    }
    ImGui::End();
}

void EditorWindow::ResetState() {
    m_state = {};
    m_rect = {};
}

void EditorWindow::UpdateState() {

    m_state.open = true;
    m_state.visible = !ImGui::IsWindowCollapsed();
    m_state.focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    m_state.hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();
    m_rect.x = pos.x;
    m_rect.y = pos.y;
    m_rect.w = size.x;
    m_rect.h = size.y;
}

}  // namespace cave
