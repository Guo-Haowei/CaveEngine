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
}

void EditorWindow::UpdateState() {
    m_state.open = true;
    m_state.visible = !ImGui::IsWindowCollapsed();
    m_state.focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    m_state.hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

    // ImVec2 pos = ImGui::GetWindowPos();
    // ImVec2 size = ImGui::GetWindowSize();
    // ImGui::GetForegroundDrawList()->AddCircle(pos, 10.f, IM_COL32(255, 0, 0, 255));
}

}  // namespace cave
