#include "viewer_tab.h"

namespace cave {

static int IdGenerator() {
    static int s_counter = 0;
    return ++s_counter;
}

ViewerTab::ViewerTab(EditorLayer& p_editor, Viewer& p_viewer)
    : m_id(IdGenerator())
    , m_editor(p_editor)
    , m_viewer(p_viewer) {

    m_title = std::format("Dummy_{}", m_id);
}

bool ViewerTab::HandleInput(const std::shared_ptr<InputEvent>& p_input_event) {
    unused(p_input_event);
    return false;
}

void ViewerTab::Draw(Scene*) {
    ImGui::Text("Dummy %s", m_title.c_str());
}

}  // namespace cave
