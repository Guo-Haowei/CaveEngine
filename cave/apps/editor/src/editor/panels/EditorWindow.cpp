#include "EditorWindow.h"

namespace cave {

EditorWindow::EditorWindow(EditorState& editor)
    : IEditorItem(editor) {}

void EditorWindow::drawUI() {
    resetState();
    if (ImGui::Begin(windowId(), nullptr, m_window_flags)) {
        updateState();
        drawUIImpl();
    }
    ImGui::End();
}

void EditorWindow::resetState() {
    m_window_state = {};
}

void EditorWindow::updateState() {
    m_window_state.open = true;
    m_window_state.visible = !ImGui::IsWindowCollapsed();
    m_window_state.focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    m_window_state.hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

    // ImVec2 pos = ImGui::GetWindowPos();
    // ImVec2 size = ImGui::GetWindowSize();
    // ImGui::GetForegroundDrawList()->AddCircle(pos, 10.f, IM_COL32(255, 0, 0, 255));
}

}  // namespace cave
