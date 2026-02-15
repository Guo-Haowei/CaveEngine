#include "EditorState.h"

#include "cave/runtime/framework/IApplication.h"

#include "cave/core/diagnostics/Profiler.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/framework/ViewManager.h"

#include "editor/services/DocumentService.h"
#include "editor/services/EditService.h"
#include "editor/services/IconCache.h"
#include "editor/services/PickingService.h"
#include "editor/services/SelectionService.h"
#include "editor/services/ShortcutService.h"
#include "editor/services/ThumbnailService.h"
#include "editor/services/Workspace.h"

// @TODO: refactor
#include <imgui/imgui_internal.h>
#include <imnodes/imnodes.h>

#include "Enums.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/InputSystem.h"
#include "engine/private/runtime/framework/IScriptService.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/ui/layout.h"

#include "editor/edit/EditObjectCmd.h"
#include "editor/EditorDvars.h"
#include "editor/panels/ContentBrowser.h"
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
    : AppState(p_app)
    , m_pie(p_app) {
    // services
    m_document_service = std::make_unique<cave::DocumentService>(*this);
    m_edit_service = std::make_unique<cave::EditService>(*this);
    m_picking_service = std::make_unique<cave::PickingService>(*this);
    m_selection_service = std::make_unique<cave::SelectionService>(*this);
    m_shortcut_service = std::make_unique<cave::ShortcutService>(*this);
    m_thumbnail_service = std::make_unique<cave::ThumbnailService>(*this);
    m_workspace = std::make_unique<cave::Workspace>(*this);
    m_icon_cache = std::make_unique<cave::IconCache>(*GetApp().GetAssetRegistry(), *GetApp().GetAssetManager());

    // panels
    m_content_browser = std::make_shared<ContentBrowser>(*this);
    m_menu_bar = std::make_shared<MenuBar>(*this);
    m_log_panel = std::make_shared<LogPanel>(*this);
    m_file_system_panel = std::make_shared<FileSystemPanel>(*this);

    AddPanel(m_log_panel);
    AddPanel(std::make_shared<RendererPanel>(*this));
    AddPanel(std::make_shared<HierarchyPanel>(*this));
    AddPanel(std::make_shared<PropertyPanel>(*this));
    AddPanel(m_content_browser);
    AddPanel(std::make_shared<RenderGraphViewer>(*this));
    AddPanel(m_file_system_panel);
}

EditorState::~EditorState() {
    m_panels.clear();
}

void EditorState::OnEnter(const StateRequest& p_args) {
    CAVE_PROFILE_EVENT();
    unused(p_args);

    ImNodes::CreateContext();

    for (auto& panel : m_panels) {
        panel->OnAttach();
    }

    SceneId edit_scene{};
    if (auto asset = DVAR_GET_STRING(last_open_asset); !asset.empty()) {
        if (auto res = Guid::Parse(asset); res.is_some()) {
            Guid guid = res.unwrap_unchecked();
            if (auto handle = m_app.GetAssetRegistry()->FindByGuid(guid); handle.is_some()) {
                AssetHandle handle_ = handle.unwrap_unchecked();
                DocId doc_id = m_document_service->OpenDoc({ guid, handle_.GetMeta()->type });
                if (IDocument* doc = m_document_service->Resolve(doc_id)) {
                    edit_scene = doc->GetPreviewScene();
                }
            }
        }
    }

    // load pie
    {
        PIEStartDesc desc{};
        desc.game_dll = "game_Debug.dll";
        desc.game_id = "chess";
        desc.edit_scene = edit_scene;

        m_pie.Start(desc);
    }
}

void EditorState::OnExit() {
    CAVE_PROFILE_EVENT();

    if (IsPlaying()) {
        LOG("@TODO: stop game module");
    }

    ImNodes::DestroyContext();

    m_pie.Stop();
}

void EditorState::Tick(const FrameTime& p_time) {
    CAVE_PROFILE_EVENT();

    BusyInfo info;
    m_thumbnail_service->Tick(p_time, info);

    if (IsPlaying()) {
        m_pie.Tick(p_time);
    }

    ImguiManager* imgui_manager = m_app.GetImguiManager();
    DEV_ASSERT(imgui_manager);

    // @TODO: refactor this
    imgui_manager->BeginFrame();

    DockSpace();
    for (auto& panel : m_panels) {
        panel->DrawUI();
    }

    m_edit_service->FlushPendingCmds();
    m_workspace->Tick();
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
#if USING(USE_LOG)
    constexpr const char* names[2] = { "Editing", "PIE" };
    LOG("EditorState::CommitModeSwitch: {} -> {}",
        names[std::to_underlying(m_state)],
        names[std::to_underlying(FlipState(m_state))]);
#endif

    switch (m_state) {
        case cave::EditorState::Mode::Editing: {
            FocusedPreviewScene preview = GetFocusedPreviewScene();
            m_pie.OnSimBegin(preview.scene_id);
        } break;
        case cave::EditorState::Mode::Playing: {
            m_pie.OnSimEnd();
        } break;
    }

    m_state = FlipState(m_state);
    m_switch_mode_requested = false;
}

void EditorState::AddPanel(std::shared_ptr<IEditorItem> p_panel) {
    m_panels.emplace_back(std::move(p_panel));
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
