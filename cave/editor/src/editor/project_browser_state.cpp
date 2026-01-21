#include "project_browser_state.h"

#include <imgui/imgui.h>

#include "engine/math/geomath.h"
#include "engine/runtime/application.h"
#include "engine/runtime/imgui_manager.h"

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

static std::vector<ProjectItem> gProjects = {
    { "Third Person Demo", "5.0" },
    { "Shooter Prototype", "4.27" },
    { "Racing Game", "5.0" },
    { "Puzzle Sandbox", "5.0EA" },
    { "Platformer", "4.26" },
    { "VR Test", "Other" },
    { "Stealth Game", "4.27" },
    { "Physics Lab", "5.0" },
};

extern auto DrawAssetCard(ImTextureID p_texture_id,
                          const char* p_name,
                          ImVec2 p_image_size) -> std::tuple<bool, bool>;

static void DrawProjectTileSimple( ) {
    ImVec2 window_size = ImGui::GetContentRegionAvail();
    constexpr float desired_icon_size = 224.f;
    int num_col = static_cast<int>(glm::floor(window_size.x / desired_icon_size));
    num_col = glm::max(1, num_col);

    ImGui::BeginTable("Inner", num_col);
    ImGui::TableNextColumn();

    ImVec2 thumbnail_size{ 196, 196 };

    for (const auto& item : gProjects) {

        // @TODO: draw stuff
        auto [hovered, clicked] = DrawAssetCard(0,
                                                item.name,
                                                thumbnail_size);
        ImGui::TableNextColumn();
    }

    ImGui::EndTable();
}

void DrawMyProjectsUI() {
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

    // Tile sizing
    const float tileW = 170.0f;
    const float tileH = 150.0f;
    ImVec2 tileSize(tileW, tileH);

    // Columns based on available width
    //float avail = ImGui::GetContentRegionAvail().x;
    //float spacing = ImGui::GetStyle().ItemSpacing.x;
    //int cols = (int)((avail + spacing) / (tileW + spacing));
    //if (cols < 1) cols = 1;

    DrawProjectTileSimple();
}

void ProjectBrowserState::Tick(float) {
    if (ImguiManager* imgui_manager = m_app.GetImguiManager()) {
        imgui_manager->BeginFrame();

        if (ImGui::Begin("Launcher")) {
            DrawMyProjectsUI();
        }
        ImGui::End();

        ImGui::Render();
    }
}

Option<StateRequest> ProjectBrowserState::PopRequest() {
    return None();
}

}  // namespace cave
