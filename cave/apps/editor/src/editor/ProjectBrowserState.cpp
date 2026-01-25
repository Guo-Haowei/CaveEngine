#include "ProjectBrowserState.h"

#include <cave/runtime/framework/IApplication.h>

#include <imgui/imgui.h>

#include <engine/private/assets/image_asset.h>
#include <engine/private/core/io/logger.h>
#include <engine/private/math/geomath.h>
#include <engine/private/runtime/framework/IAssetManager.h>
#include <engine/private/runtime/framework/AssetRegistry.h>
#include <engine/private/runtime/framework/BootLoadPipeline.h>
#include <engine/private/runtime/framework/ImGuiManager.h>
#include <engine/private/runtime/framework/TaskManager.h>
#include <engine/private/ui/layout.h>

#include "editor/widgets/Image.h"

namespace fs = std::filesystem;

namespace cave {

ProjectBrowserState::ProjectBrowserState(IApplication& p_app)
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
    std::shared_ptr<ImageAsset> image = IAssetManager::GetSingleton().FindImage("scene@256x256.png");
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
            m_app.RequestProject(item.path);
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

        ui::DockSpace({ "DockSpaceRoot",
                        nullptr,
                        [this]() {
                            DrawSideBar();
                        } });

        if (ImGui::Begin("Recent Projects")) {
            DrawUI();
        }
        ImGui::End();

        if (ImGui::Begin("Settings")) {
        }
        ImGui::End();

        if (ImGui::Begin("Project Location")) {
            if (m_request_fired) {
                TaskSnapshot root = m_app.GetBootLoadPipeline().RootSnapshot();

                if (root.indeterminate) {
                    ImGui::ProgressBar(-1.0f, ImVec2(-1.0f, 0.0f));
                } else {
                    ImGui::ProgressBar(root.progress01, ImVec2(-1.0f, 0.0f));
                }
            }
        }
        ImGui::End();

        ImGui::Render();
    }
}

void ProjectBrowserState::DrawSideBar() {
    const std::vector<LogEvent>& logs = CompositeLogger::GetSingleton().GetAllLogs();
    if (logs.empty()) {
        return;
    }

    const LogEvent& log = logs.back();

    const char* ptr1 = log.message.c_str();
    const char* ptr2 = strchr(ptr1, ']');
    ptr2 = ptr2 ? (ptr2 + 1) : ptr1;

    TaskSnapshot root = m_app.GetBootLoadPipeline().RootSnapshot();

    ImGui::Text("[%d%%] %s", static_cast<int>(root.progress01 * 100), ptr2);
}

Option<StateRequest> ProjectBrowserState::PopRequest() {
    if (!m_request_fired) {
        return None();
    }

    if (m_app.GetTaskManager()->HasPendingWork()) {
        return None();
    }

    auto request = m_request;
    m_request = None();
    return request;
}

}  // namespace cave
