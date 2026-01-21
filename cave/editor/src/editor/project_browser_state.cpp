#include "project_browser_state.h"

#include <imgui/imgui.h>

#include "engine/math/geomath.h"
#include "engine/runtime/application.h"
#include "engine/runtime/imgui_manager.h"

#include "editor/widgets/image.h"

namespace cave {

ProjectBrowserState::ProjectBrowserState(Application& p_app)
    : AppState(p_app) {
}

void ProjectBrowserState::OnEnter(const StateRequest& p_args) {
    unused(p_args);
}

void ProjectBrowserState::OnExit() {
}

struct ProjectItem {
    const char* name;
    const char* version;
};

static std::vector<ProjectItem> s_projects = {
    { "Third Person Demo", "5.0" },
    { "Shooter Prototype", "4.27" },
    { "Racing Game", "5.0" },
    { "Puzzle Sandbox", "5.0EA" },
    { "Platformer", "4.26" },
    { "VR Test", "Other" },
    { "Stealth Game", "4.27" },
    { "Physics Lab", "5.0" },
};

static bool DrawProjects() {
    ImVec2 window_size = ImGui::GetContentRegionAvail();
    constexpr float desired_icon_size = 224.f;
    int num_col = static_cast<int>(glm::floor(window_size.x / desired_icon_size));
    num_col = glm::max(1, num_col);

    ImGui::BeginTable("Inner", num_col);
    ImGui::TableNextColumn();

    Vector2f thumbnail_size(196);

    bool any_click = false;
    for (const auto& item : s_projects) {
        auto [hovered, clicked] = ui::AssetCard(0, item.name, thumbnail_size);
        any_click = any_click || clicked;
        ImGui::TableNextColumn();
    }

    ImGui::EndTable();
    return any_click;
}

void ProjectBrowserState::DrawUI() {
    static int selectedIndex = -1;
    static char search[128] = "";

    ImGui::SeparatorText("MY PROJECTS");

    // Right-aligned search box
    float searchW = 260.0f;
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - searchW);
    ImGui::SetNextItemWidth(searchW);
    ImGui::InputTextWithHint("##search", "Search Projects", search, sizeof(search));

    ImGui::Spacing();

    bool any_click = DrawProjects();
    if (any_click) {
        m_request = Some(StateRequest{ AppStateId::Editor });
    }
}

void ProjectBrowserState::Tick(float) {
    if (ImguiManager* imgui_manager = m_app.GetImguiManager()) {
        imgui_manager->BeginFrame();

        if (ImGui::Begin("Launcher")) {
            DrawUI();
        }
        ImGui::End();

        ImGui::Render();
    }
}

Option<StateRequest> ProjectBrowserState::PopRequest() {
    auto request = m_request;
    m_request = None();
    return request;
}

}  // namespace cave
