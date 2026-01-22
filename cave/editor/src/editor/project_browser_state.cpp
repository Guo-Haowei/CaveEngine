#include "project_browser_state.h"

#include <imgui/imgui.h>

#include "engine/assets/image_asset.h"
#include "engine/math/geomath.h"
#include "engine/runtime/application.h"
#include "engine/runtime/asset_manager_interface.h"
#include "engine/runtime/asset_registry.h"
#include "engine/runtime/boot_load_pipeline.h"
#include "engine/runtime/imgui_manager.h"
#include "engine/runtime/task_manager.h"

#include "editor/widgets/image.h"
#include "engine/ui/layout.h"

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

    if (m_request_fired) {
        TaskSnapshot root = m_app.GetBootLoadPipeline().RootSnapshot();

        if (root.indeterminate) {
            ImGui::ProgressBar(-1.0f, ImVec2(-1.0f, 0.0f));
        } else {
            ImGui::ProgressBar(root.progress01, ImVec2(-1.0f, 0.0f));
        }
    }
}

void ProjectBrowserState::Tick(float) {
    if (ImguiManager* imgui_manager = m_app.GetImguiManager()) {
        imgui_manager->BeginFrame();

        if (BeginFullscreenWindow("Project Browser")) {
            DrawUI();
        }
        EndFullscreenWindow();

        ImGui::Render();
    }
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

#if 0
class BootLoadPipeline {
public:
    BootLoadPipeline(TaskManager& tm, IAssetManager& am, AssetRegistry& reg)
        : m_tm(tm), m_am(am), m_reg(reg) {}

    void Start(const std::filesystem::path& project_path) {
        m_children.clear();

        // 1) Request project (registry scanning / indexing) as a task
        m_children.push_back(SubmitRequestProject(project_path));

        // 2) Preload essential assets as a task (uses registry list)
        m_children.push_back(SubmitPreloadEssentials());

        // 3) Optional: import / compile / warmup
        // m_children.push_back(...)

        TaskGroupSpec group;
        group.name = "Boot";
        group.children = m_children;
        group.weights = { 0.3f, 0.7f };

        m_root = m_tm.SubmitGroup(std::move(group), TaskPriority::High);
    }

    TaskSnapshot RootSnapshot() const { return m_tm.GetSnapshot(m_root); }

    std::vector<TaskSnapshot> ChildSnapshots() const {
        std::vector<TaskSnapshot> out;
        out.reserve(m_children.size());
        for (auto id : m_children) out.push_back(m_tm.GetSnapshot(id));
        return out;
    }

    TaskId RootId() const { return m_root; }

private:
    TaskId SubmitRequestProject(const std::filesystem::path& project_path) {
        struct RequestProjectTask final : IAsyncTask {
            AssetRegistry& reg;
            std::filesystem::path path;
            RequestProjectTask(AssetRegistry& r, std::filesystem::path p) : reg(r), path(std::move(p)) {}

            void Run(TaskContext& ctx) override {
                ctx.SetIndeterminate(true);
                auto res = reg.RequestProject(path);
                if (res.is_err()) ctx.Fail(res.err().ToString());
                ctx.SetIndeterminate(false);
                ctx.SetProgress(1.0f);
            }
        };

        return m_tm.Submit(std::make_unique<RequestProjectTask>(m_reg, project_path));
    }

    TaskId SubmitPreloadEssentials() {
        // however you decide essentials (by type, tag, persistent list, etc.)
        std::vector<Guid> essentials = GatherEssentialsFromRegistry();

        AssetLoadRequest req;
        req.name = "Preload essentials";
        req.guids = std::move(essentials);
        req.high_priority = true;

        return m_am.SubmitLoadAssets(m_tm, req);
    }

    std::vector<Guid> GatherEssentialsFromRegistry() {
        // placeholder – implement however you already do it
        std::vector<Guid> out;
        // e.g. reg.GetAssetsOfType(Image/Shader/etc.) -> guids
        return out;
    }

private:
    TaskManager& m_tm;
    IAssetManager& m_am;
    AssetRegistry& m_reg;

    TaskId m_root = kInvalidTaskId;
    std::vector<TaskId> m_children;
};
#endif

}  // namespace cave
