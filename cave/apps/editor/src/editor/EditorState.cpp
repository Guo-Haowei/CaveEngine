#include "EditorState.h"

#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/gameplay/IGameMode.h"

#include "engine/private/core/debugger/Profiler.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/framework/ViewManager.h"

#include "editor/services/DocumentService.h"
#include "editor/services/EditService.h"
#include "editor/services/PickingService.h"
#include "editor/services/SelectionService.h"
#include "editor/services/ShortcutService.h"
#include "editor/services/Workspace.h"

// @TODO: refactor
#include <imgui/imgui_internal.h>
#include <imnodes/imnodes.h>

#include "Enums.h"
#include "engine/private/assets/image_asset.h"
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/InputSystem.h"
#include "engine/private/runtime/scene/ISceneRegistry.h"
#include "engine/private/runtime/framework/RuntimeHost.h"
#include "engine/private/runtime/framework/IScriptManager.h"
#include "engine/private/ui/layout.h"

#include "editor/edit/EditObjectCmd.h"
#include "editor/EditorDvars.h"
#include "editor/panels/AssetInspector.h"
#include "editor/panels/FileSystemPanel.h"
#include "editor/panels/HierarchyPanel.h"
#include "editor/panels/LogPanel.h"
#include "editor/panels/MenuBar.h"
#include "editor/panels/PropertyPanel.h"
#include "editor/panels/RenderGraphViewer.h"
#include "editor/panels/RendererPanel.h"
#include "editor/widgets/Image.h"

namespace cave {

EditorState::EditorState(IApplication& p_app)
    : AppState(p_app) {
    // services
    m_document_service = std::make_unique<cave::DocumentService>(*this);
    m_edit_service = std::make_unique<cave::EditService>(*this);
    m_picking_service = std::make_unique<cave::PickingService>(*this);
    m_selection_service = std::make_unique<cave::SelectionService>(*this);
    m_shortcut_service = std::make_unique<cave::ShortcutService>(*this);
    m_workspace = std::make_shared<cave::Workspace>(*this);

    // runtime
    m_runtime_host = std::make_unique<RuntimeHost>(p_app);

    // panels
    m_asset_inspector = std::make_shared<AssetInspector>(*this);
    m_menu_bar = std::make_shared<MenuBar>(*this);
    m_log_panel = std::make_shared<LogPanel>(*this);
    m_file_system_panel = std::make_shared<FileSystemPanel>(*this);

    AddPanel(m_log_panel);
    AddPanel(std::make_shared<RendererPanel>(*this));
    AddPanel(std::make_shared<HierarchyPanel>(*this));
    AddPanel(std::make_shared<PropertyPanel>(*this));
    AddPanel(m_asset_inspector);
    AddPanel(std::make_shared<RenderGraphViewer>(*this));
    AddPanel(m_file_system_panel);
}

EditorState::~EditorState() {
    m_panels.clear();
}

void EditorState::OnEnter(const StateRequest& p_args) {
    CAVE_PROFILE_EVENT();
    unused(p_args);

    const char* module_name = "game_Debug.dll";
    LoadGameModule(module_name, m_module);

    if (m_module.api && m_module.api->RegisterGame) {
        GameLoadArgs args{};
        m_module.api->RegisterGame(m_app, args);
    }

    ImNodes::CreateContext();

    {
        // @TODO: get rid of this
        auto handle = AssetRegistry::GetSingleton().FindByPath<ImageAsset>("@persist://textures/checkerboard");
        if (handle.is_some()) {
            context.checkerboard = handle.unwrap_unchecked().Wait();
        }
    }

    for (auto& panel : m_panels) {
        panel->OnAttach();
    }

    if (auto asset = DVAR_GET_STRING(last_open_asset); !asset.empty()) {
        if (auto res = Guid::Parse(asset); res.is_some()) {
            Guid guid = res.unwrap_unchecked();
            if (auto handle = m_app.GetAssetRegistry()->FindByGuid(guid); handle.is_some()) {
                AssetHandle handle_ = handle.unwrap_unchecked();
                m_document_service->OpenDoc({ guid, handle_.GetMeta()->type });
            }
        }
    }
}

void EditorState::OnExit() {
    CAVE_PROFILE_EVENT();

    if (IsPlaying()) {
        m_runtime_host->Stop();
    }

    ImNodes::DestroyContext();

    UnloadGameModule(m_module);
}

void EditorState::Tick(float p_dt) {
    CAVE_PROFILE_EVENT();

    if (IsPlaying()) {
        GameFrameTime frame;
        frame.dt = p_dt;
        m_runtime_host->Tick(frame);
    }

    ImguiManager* imgui_manager = m_app.GetImguiManager();
    DEV_ASSERT(imgui_manager);

    // @TODO: refactor this
    imgui_manager->BeginFrame();

    DockSpace();
    for (auto& it : m_panels) it->DrawUI();

    m_edit_service->FlushPendingCmds();
    m_workspace->Tick(p_dt);
    m_picking_service->Tick();

    ImGui::Render();

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
            FocusedPreviewScene preview = GetFocusedPreviewScene();

            RuntimeStartParams params(std::move(SceneSource::FromExisting(preview.scene_id)));
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

void EditorState::AddPanel(std::shared_ptr<IEditorItem> p_panel) {
    m_panels.emplace_back(p_panel);
}

void EditorState::DockSpace() {
    CAVE_PROFILE_EVENT();

    ui::DockSpace({
        "DockSpace Demo",
        [this]() { m_menu_bar->DrawUI(); },
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

FocusedPreviewScene EditorState::GetFocusedPreviewScene() {
    FocusedPreviewScene ret;
    if (Tab* tab = m_workspace->GetFocusedTab()) {
        ret.doc_id = tab->GetDocId();
        if (IDocument* doc = m_document_service->Resolve(ret.doc_id)) {
            ret.scene_id = doc->GetPreviewScene();
            ret.scene = m_app.GetSceneRegistry()->Resolve(ret.scene_id);
        }
    }
    return ret;
}

void EditorState::OpenAddEntityPopup(ecs::Entity p_parent) {
    if (ImGui::BeginMenu("Add")) {
        FocusedPreviewScene preview = GetFocusedPreviewScene();
        DocId doc_id = preview.doc_id;
#define ENTITY_TYPE(NAME, SEP)                                                           \
    if (ImGui::MenuItem(#NAME)) {                                                        \
        auto cmd = std::make_unique<AddObjectCmd>(GetApp(), p_parent, EntityType::NAME); \
        m_edit_service->Submit(doc_id, std::move(cmd));                                  \
    }                                                                                    \
    if constexpr (SEP) {                                                                 \
        ImGui::Separator();                                                              \
    }
        ENTITY_TYPE_LIST
#undef ENTITY_TYPE
        ImGui::EndMenu();
    }
}

}  // namespace cave
