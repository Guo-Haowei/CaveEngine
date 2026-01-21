#include "project_browser_state.h"

#include <imgui/imgui.h>

#include "engine/assets/image_asset.h"
#include "engine/math/geomath.h"
#include "engine/runtime/application.h"
#include "engine/runtime/asset_registry.h"
#include "engine/runtime/imgui_manager.h"

#include "editor/editor_asset_manager.h"
#include "editor/widgets/image.h"

namespace fs = std::filesystem;

namespace cave {

static void BeginFullscreenWindow(const char* name) {
    ImGuiViewport* vp = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::SetNextWindowViewport(vp->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoDocking;

    ImGui::Begin(name, nullptr, flags);
}

static void EndFullscreenWindow() {
    ImGui::End();
    ImGui::PopStyleVar(3);
}

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
    constexpr float desired_icon_size = 296.f;
    int num_col = static_cast<int>(glm::floor(window_size.x / desired_icon_size));
    num_col = glm::max(1, num_col);

    ImGui::BeginTable("Inner", num_col);
    ImGui::TableNextColumn();

    Vector2f thumbnail_size(256);

    // @TODO: use actual image
    auto& asset_manager = static_cast<EditorAssetManager&>(IAssetManager::GetSingleton());
    std::shared_ptr<ImageAsset> image = asset_manager.FindImage("scene@256x256.png");
    GpuTexture* texture = image ? image->gpu_texture.get() : nullptr;

    for (const auto& item : m_projects) {
        auto [hovered, clicked] = ui::AssetCard(texture ? texture->GetHandle() : 0,
                                                item.name.c_str(),
                                                thumbnail_size);
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

        BeginFullscreenWindow("Project Browser");
        DrawUI();
        EndFullscreenWindow();

        ImGui::Render();
    }
}

Option<StateRequest> ProjectBrowserState::PopRequest() {
    auto request = m_request;
    m_request = None();
    return request;
}

}  // namespace cave
