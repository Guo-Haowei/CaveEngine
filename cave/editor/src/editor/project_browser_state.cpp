#include "project_browser_state.h"

#include <imgui/imgui.h>

#include "engine/math/geomath.h"
#include "engine/runtime/application.h"
#include "engine/runtime/asset_registry.h"
#include "engine/runtime/imgui_manager.h"

#include "editor/widgets/image.h"

namespace fs = std::filesystem;

namespace cave {

ProjectBrowserState::ProjectBrowserState(Application& p_app)
    : AppState(p_app) {
}

void ProjectBrowserState::OnEnter(const StateRequest&) {
    fs::path project_root{ ROOT_FOLDER "projects" };
    if (!fs::exists(project_root) || !fs::is_directory(project_root)) {
        return;
    }

    for (const auto& entry : fs::directory_iterator(project_root)) {
        if (!entry.is_directory()) {
            continue;
        }

        fs::path project_file = entry.path() / "project.yaml";

        if (!fs::exists(project_file)) {
            continue;
        }

        std::string name = entry.path().filename().string();
        std::string path = entry.path().string();

        std::replace(path.begin(), path.end(), '/', '\\');

        m_projects.push_back({ std::move(name), std::move(path) });
    }
}

void ProjectBrowserState::OnExit() {
}

void ProjectBrowserState::DrawRecentProjects() {
    ImVec2 window_size = ImGui::GetContentRegionAvail();
    constexpr float desired_icon_size = 224.f;
    int num_col = static_cast<int>(glm::floor(window_size.x / desired_icon_size));
    num_col = glm::max(1, num_col);

    ImGui::BeginTable("Inner", num_col);
    ImGui::TableNextColumn();

    Vector2f thumbnail_size(196);

    for (const auto& item : m_projects) {
        auto [hovered, clicked] = ui::AssetCard(0, item.name.c_str(), thumbnail_size);
        ImGui::TableNextColumn();

        if (hovered) {
            ImGui::BeginTooltip();
            ImGui::Text("path: %s", item.path.c_str());
            ImGui::EndTooltip();
        }

        if (clicked && !m_request_fired) {
            m_request = Some(StateRequest{ AppStateId::Editor, item.path });
            m_app.LoadProjectAsync(item.path);
            m_request_fired = true;
        }
    }

    ImGui::EndTable();
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

    DrawRecentProjects();
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
