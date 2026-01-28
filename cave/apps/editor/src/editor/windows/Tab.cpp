#include "Tab.h"

namespace cave {

Tab::Tab(EditorState& p_editor)
    : EditorWindow(p_editor) {}

void Tab::UpdateInternal(float p_dt) {
    unused(p_dt);

    ImGui::Text("%s", m_window_id.c_str());
    ImGui::Text("ID: %u", m_idx);
}

void Tab::SetTitleAndId(std::string_view p_title, uint32_t p_idx) {
    m_idx = p_idx;
    m_title = p_title.empty() ? "Untitled" : p_title;
    m_window_id = std::format("{}###WorkspaceTab{}", m_title, m_idx);
}

}  // namespace cave
