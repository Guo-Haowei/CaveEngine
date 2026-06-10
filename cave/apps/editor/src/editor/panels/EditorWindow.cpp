#include "EditorWindow.h"

namespace cave {

EditorWindow::EditorWindow(EditorState& editor)
    : IEditorItem(editor) {}

void EditorWindow::drawUI() {
    resetState();
    if (ImGui::Begin(windowId(), nullptr, flags_)) {
        updateState();
        drawUIImpl();
    }
    ImGui::End();
}

void EditorWindow::resetState() {
    state_ = {};
}

void EditorWindow::updateState() {
    state_.open = true;
    state_.visible = !ImGui::IsWindowCollapsed();
    state_.focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    state_.hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

    // ImVec2 pos = ImGui::GetWindowPos();
    // ImVec2 size = ImGui::GetWindowSize();
    // ImGui::GetForegroundDrawList()->AddCircle(pos, 10.f, IM_COL32(255, 0, 0, 255));
}

}  // namespace cave
