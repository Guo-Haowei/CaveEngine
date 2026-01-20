#include "project_browser_state.h"

#include <imgui/imgui.h>

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

static bool DrawProjectTileSimple(
    const ProjectItem& p,
    int index,
    bool selected,
    ImVec2 tileSize) {
    ImGui::PushID(index);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 end = ImVec2(pos.x + tileSize.x, pos.y + tileSize.y);

    float rounding = 6.0f;

    ImU32 bg = ImGui::GetColorU32(ImGuiCol_ChildBg);
    ImU32 border = ImGui::GetColorU32(ImGuiCol_Border);
    ImU32 hover = ImGui::GetColorU32(ImVec4(0.30f, 0.30f, 0.34f, 1.0f));
    ImU32 accent = ImGui::GetColorU32(ImVec4(0.95f, 0.72f, 0.06f, 1.0f));  // Epic-ish

    // Card background + border
    dl->AddRectFilled(pos, end, bg, rounding);
    dl->AddRect(pos, end, border, rounding);

    // Click area
    ImGui::InvisibleButton("##tile", tileSize);
    [[maybe_unused]] bool hovered = ImGui::IsItemHovered();
    [[maybe_unused]] bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
    bool dbl = hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

    // Hover / selected outline
    if (hovered)
        dl->AddRect(pos, end, hover, rounding, 0, 2.0f);
    if (selected)
        dl->AddRect(pos, end, accent, rounding, 0, 2.5f);

    // Inner content
    ImVec2 pad(10, 10);
    ImGui::SetCursorScreenPos(ImVec2(pos.x + pad.x, pos.y + pad.y));

    // Placeholder thumbnail box
    ImVec2 thumbSize(tileSize.x - 20.0f, 60.0f);
    ImVec2 t0 = ImGui::GetCursorScreenPos();
    ImVec2 t1 = ImVec2(t0.x + thumbSize.x, t0.y + thumbSize.y);
    dl->AddRectFilled(t0, t1, ImGui::GetColorU32(ImVec4(0.12f, 0.12f, 0.14f, 1.0f)), 4.0f);
    dl->AddRect(t0, t1, border, 4.0f);
    ImGui::Dummy(thumbSize);

    // Version badge (top-right of placeholder)
    ImVec2 textSz = ImGui::CalcTextSize(p.version);
    ImVec2 bPad(6, 3);

    ImVec2 b1 = ImVec2(t1.x - 6, t0.y + 6);
    ImVec2 b0 = ImVec2(b1.x - (textSz.x + bPad.x * 2), b1.y);
    ImVec2 b2 = ImVec2(b1.x, b1.y + textSz.y + bPad.y * 2);

    dl->AddRectFilled(b0, b2, ImGui::GetColorU32(ImVec4(0.10f, 0.10f, 0.11f, 0.95f)), 4.0f);
    dl->AddRect(b0, b2, border, 4.0f);
    dl->AddText(ImVec2(b0.x + bPad.x, b0.y + bPad.y),
                ImGui::GetColorU32(ImGuiCol_Text),
                p.version);

    // Name label
    ImGui::SetCursorScreenPos(ImVec2(pos.x + pad.x, t1.y + 8));
    ImGui::PushTextWrapPos(pos.x + tileSize.x - pad.x);
    ImGui::TextUnformatted(p.name);
    ImGui::PopTextWrapPos();

    // Advance cursor
    ImGui::SetCursorScreenPos(ImVec2(
        pos.x,
        pos.y + tileSize.y + ImGui::GetStyle().ItemSpacing.y));

    ImGui::PopID();

    // Return true on double-click (open)
    return dbl;
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
    float avail = ImGui::GetContentRegionAvail().x;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    int cols = (int)((avail + spacing) / (tileW + spacing));
    if (cols < 1) cols = 1;

    int col = 0;
    for (int i = 0; i < (int)gProjects.size(); ++i) {
        // Simple case-insensitive filter
        if (search[0] != 0) {
            std::string name = gProjects[i].name;
            std::string q = search;
            std::transform(name.begin(), name.end(), name.begin(),
                           [](unsigned char c) {
                               return (char)std::tolower(c);
                           });

            std::transform(q.begin(), q.end(), q.begin(),
                           [](unsigned char c) {
                               return (char)std::tolower(c);
                           });
            if (name.find(q) == std::string::npos)
                continue;
        }

        bool isSel = (selectedIndex == i);

        [[maybe_unused]]
        bool opened = DrawProjectTileSimple(
            gProjects[i],
            i,
            isSel,
            tileSize);

        // Selection logic
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            selectedIndex = i;
            ImGui::Text("Open project: %s", gProjects[i].name);  // placeholder action
        } else if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            selectedIndex = i;
        }

        col++;
        if (col < cols)
            ImGui::SameLine();
        else
            col = 0;
    }
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
