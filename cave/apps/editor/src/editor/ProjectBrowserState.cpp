#include "ProjectBrowserState.h"

#include <imgui/imgui.h>

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/core/diagnostics/log_sink/CompositeLogger.h"
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/BootLoadPipeline.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/framework/TaskManager.h"
#include "engine/private/runtime/projects/ProjectManager.h"
#include "engine/private/serialization/yaml_include.h"
#include "engine/private/ui/layout.h"

#include "editor/widgets/Image.h"

namespace cave {

namespace fs = std::filesystem;

static auto scanProjects(const std::filesystem::path& root) -> std::vector<ProjectInfo>;

ProjectBrowserState::ProjectBrowserState(IApplication& app)
    : AppState(app)
    , project_manager_(app.services().projectManager())
    , debug_id_(MakeDebugId(this)) {
}

void ProjectBrowserState::onEnter(const StateRequest&) {
    project_list_ = scanProjects(fs::path(ROOT_FOLDER) / "projects");
}

void ProjectBrowserState::onExit() {
}

void ProjectBrowserState::drawRecentProjects() {
    ImVec2 window_size = ImGui::GetContentRegionAvail();
    constexpr float desired_icon_size = 296.f;
    int num_col = static_cast<int>(glm::floor(window_size.x / desired_icon_size));
    num_col = glm::max(1, num_col);

    ImGui::BeginTable("Inner", num_col);
    ImGui::TableNextColumn();

    math::Vector2f thumbnail_size(256);

    // @TODO: use actual image
    std::shared_ptr<ImageAsset> image = IAssetManager::GetSingleton().FindImage("scene@256x256.png");
    GpuTexture* texture = image ? image->gpu_texture.get() : nullptr;

    for (const ProjectInfo& project : project_list_) {
        auto [hovered, clicked] = ui::AssetCard(texture ? texture->GetHandle() : 0,
                                                project.name.c_str(),
                                                thumbnail_size);
        ImGui::TableNextColumn();

        if (hovered) {
            ImGui::BeginTooltip();
            ImGui::Text("version: %d", project.version);
            ImGui::Text("path: %s", project.path.c_str());
            ImGui::Text("start_scene: %s", project.start_scene.c_str());
            ImGui::EndTooltip();
        }

        if (clicked && !request_fired_) {
            request_ = Some(StateRequest{
                .next = AppStateId::Editor,
                .arg0 = project.name,
                .arg1 = project.start_scene,
            });
            project_manager_.loadProject(project);
            request_fired_ = true;
        }
    }

    ImGui::EndTable();
}

void ProjectBrowserState::drawUI() {
    static int selectedIndex = -1;
    static char search[128] = "";

    ImGui::SeparatorText("MY PROJECTS");

    // Right-aligned search box
    float search_w = 260.0f;
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - search_w);
    ImGui::SetNextItemWidth(search_w);
    ImGui::InputTextWithHint("##search", "Search Projects", search, sizeof(search));

    ImGui::Spacing();

    drawRecentProjects();
}

void ProjectBrowserState::tick(const FrameTime&) {
    if (ImguiManager* imgui_manager = app_.GetImguiManager()) {
        imgui_manager->BeginFrame();

        ui::DockSpace({ "DockSpaceRoot",
                        nullptr,
                        [this]() {
                            drawSideBar();
                        } });

        if (ImGui::Begin("Recent Projects")) {
            drawUI();
        }
        ImGui::End();

        if (ImGui::Begin("Settings")) {
        }
        ImGui::End();

        if (ImGui::Begin("Project Location")) {
            if (request_fired_) {
                TaskSnapshot root = project_manager_.snapshot();

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

void ProjectBrowserState::drawSideBar() {
    const std::vector<LogEvent>& logs = CompositeLogger::GetSingleton().GetAllLogs();
    if (logs.empty()) {
        return;
    }

    const LogEvent& log = logs.back();

    const char* ptr1 = log.message.c_str();
    const char* ptr2 = strchr(ptr1, ']');
    ptr2 = ptr2 ? (ptr2 + 1) : ptr1;

    TaskSnapshot root = project_manager_.snapshot();

    ImGui::Text("[%d%%] %s", static_cast<int>(root.progress01 * 100), ptr2);
}

Option<StateRequest> ProjectBrowserState::popRequest() {
    if (!request_fired_) {
        return None();
    }

    if (app_.services().taskManager().HasPendingWork()) {
        return None();
    }

    auto request = request_;
    request_ = None();
    return request;
}

template<typename T>
static bool tryReadYaml(const YAML::Node& node, const char* key, T& out) {
    try {
        const YAML::Node value = node[key];
        if (!value || value.IsNull()) {
            return false;
        }

        out = value.as<T>();
        return true;
    } catch (const YAML::Exception&) {
        return false;
    }
}

static bool parseProject(const std::filesystem::path& path, ProjectInfo& out_info) {
    YAML::Node node;
    if (auto res = LoadYaml(path.string(), node); !res) {
        DEV_ASSERT(0 && "TODO: error reporting");
        return false;
    }

    tryReadYaml(node, "name", out_info.name);
    tryReadYaml(node, "start_scene", out_info.start_scene);
    tryReadYaml(node, "thumbnail", out_info.thumbnail);
    tryReadYaml(node, "is_2d", out_info.is_2d);
    return true;
}

auto scanProjects(const std::filesystem::path& root) -> std::vector<ProjectInfo> {
    std::vector<ProjectInfo> projects;

    if (fs::exists(root) && fs::is_directory(root)) {
        for (const auto& entry : fs::directory_iterator(root)) {
            if (!entry.is_directory()) {
                continue;
            }

            fs::path project_file = entry.path() / "project.yaml";

            if (!fs::exists(project_file)) {
                continue;
            }

            std::string path = entry.path().string();
            std::replace(path.begin(), path.end(), '/', '\\');

            LOG_TRACE(LogChannel::Asset, "Found @{}", path);

            ProjectInfo info;

            if (!parseProject(project_file, info)) {
                continue;
            }

            info.path = std::move(path);

            projects.emplace_back(std::move(info));
        }
    }

    return projects;
}

}  // namespace cave
