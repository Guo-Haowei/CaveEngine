#include "EditorState.h"

#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/gameplay/IGameMode.h"

#include "engine/private/debugger/profiler.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/framework/ViewportManager.h"

#include "editor/services/EditService.h"
#include "editor/services/ShortcutService.h"

// @TODO: refactor
#include <imgui/imgui_internal.h>
#include <imnodes/imnodes.h>

#include "engine/private/assets/image_asset.h"
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/renderer/graphics_manager.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/InputSystem.h"
#include "engine/private/runtime/scene/SceneManager.h"
#include "engine/private/runtime/framework/RuntimeHost.h"
#include "engine/private/runtime/framework/IScriptManager.h"
#include "engine/private/ui/layout.h"

#include "editor/document/Document.h"
#include "editor/EditorDvars.h"
#include "editor/panels/AssetInspector.h"
#include "editor/panels/FileSystemPanel.h"
#include "editor/panels/HierarchyPanel.h"
#include "editor/panels/LogPanel.h"
#include "editor/panels/MenuBar.h"
#include "editor/panels/PropertyPanel.h"
#include "editor/panels/RenderGraphViewer.h"
#include "editor/panels/RendererPanel.h"
#include "editor/viewer/Viewer.h"
#include "editor/viewer/ViewerTab.h"
#include "editor/widgets/Image.h"

namespace cave {

EditorState::EditorState(IApplication& p_app)
    : AppState(p_app) {
    // shortcut
    m_edit_service = std::make_unique<EditService>(*this);
    m_shortcut_service = std::make_unique<ShortcutService>(*this);

    // runtime
    m_runtime_host = std::make_unique<RuntimeHost>(p_app);

    // panels
    m_asset_inspector = std::make_shared<AssetInspector>(*this);
    m_menu_bar = std::make_shared<MenuBar>(*this);
    m_viewer = std::make_shared<Viewer>(*this);
    m_log_panel = std::make_shared<LogPanel>(*this);
    m_file_system_panel = std::make_shared<FileSystemPanel>(*this);

    AddPanel(m_log_panel);
    AddPanel(std::make_shared<RendererPanel>(*this));
    AddPanel(std::make_shared<HierarchyPanel>(*this));
    AddPanel(std::make_shared<PropertyPanel>(*this));
    AddPanel(m_viewer);
    AddPanel(m_asset_inspector);
    AddPanel(std::make_shared<RenderGraphViewer>(*this));
    AddPanel(m_file_system_panel);
}

EditorState::~EditorState() {
    m_panels.clear();
}

void EditorState::OnEnter(const StateRequest& p_args) {
    unused(p_args);

    const char* module_name = "game_Debug.dll";
    LoadGameModule(module_name, m_module);

    if (m_module.api && m_module.api->RegisterGame) {
        GameLoadArgs args{};
        m_module.api->RegisterGame(m_app, args);
    }

    ImNodes::CreateContext();

    auto handle = AssetRegistry::GetSingleton().FindByPath<ImageAsset>("@persist://textures/checkerboard");
    if (handle.is_some()) {
        context.checkerboard = handle.unwrap_unchecked().Wait();
    }

    for (auto& panel : m_panels) {
        panel->OnAttach();
    }

    if (auto asset = DVAR_GET_STRING(last_open_asset); !asset.empty()) {
        if (auto res = Guid::Parse(asset); res.is_some()) {
            Guid guid = res.unwrap_unchecked();
            m_edit_service->CommandInspectAsset(guid);
        }
    }
}

void EditorState::OnExit() {
    if (IsPlaying()) {
        m_runtime_host->Stop();
    }

    m_app.GetViewportManager()->ClearViewport();

    ImNodes::DestroyContext();

    UnloadGameModule(m_module);
}

void EditorState::Tick(float p_timestep) {
    CAVE_PROFILE_EVENT();

    if (IsPlaying()) {
        GameFrameTime frame;
        frame.dt = p_timestep;
        m_runtime_host->Tick(frame);
    }

    ImguiManager* imgui_manager = m_app.GetImguiManager();
    DEV_ASSERT(imgui_manager);

    // @TODO: refactor this
    imgui_manager->BeginFrame();

    DockSpace();
    for (auto& it : m_panels) {
        it->Update(p_timestep);
    }

    {
        CAVE_PROFILE_EVENT("ImGui::Render");
        ImGui::Render();
    }

    m_edit_service->FlushCommand(nullptr);
    CommitModeSwitch();
}

void EditorState::RequestModeSwitch() {
    m_switch_mode_requested = true;
}

void EditorState::CommitModeSwitch() {
    if (!m_switch_mode_requested) {
        return;
    }

    // @TODO: refactor
#if 1
    {
        const char* names[2] = { "Editing", "PIE" };
        LOG("EditorState::CommitModeSwitch: {} -> {}",
            names[std::to_underlying(m_state)],
            names[std::to_underlying(FlipState(m_state))]);
    }
#endif

    switch (m_state) {
        case cave::EditorState::Mode::Editing: {
            ViewerTab* tab = m_viewer->GetActiveTab();
            DEV_ASSERT(tab);
            RuntimeStartParams params(std::move(SceneSource::FromExisting(tab->GetSceneId())));
            params.game_mode_id = "chess";
            params.mode = RuntimeStartParams::Mode::PIE;
            m_runtime_host->Start(params);
        } break;
        case cave::EditorState::Mode::Playing: {
            m_runtime_host->Stop();
        } break;
    }

    m_state = FlipState(m_state);
    m_switch_mode_requested = false;
}

void EditorState::AddPanel(std::shared_ptr<EditorItem> p_panel) {
    m_panels.emplace_back(p_panel);
}

void EditorState::DockSpace() {
    CAVE_PROFILE_EVENT();

    ui::DockSpace({
        "DockSpace Demo",
        [this]() { m_menu_bar->Update(0.0f); },
        [this]() {
            CompositeLogger& logger = CompositeLogger::GetSingleton();
            const uint32_t error_count = static_cast<uint32_t>(logger.GetErrorLogs().size());
            const uint32_t warning_count = static_cast<uint32_t>(logger.GetWarningLogs().size());

            ui::ErrorIcon();

            ImGui::SameLine();
            ImGui::Text(" %u Error(s)", error_count);

            ImGui::SameLine();
            ui::WarningIcon();

            ImGui::SameLine();
            ImGui::Text(" %u Warning(s)", warning_count);
        },
    });

    return;
}

}  // namespace cave
