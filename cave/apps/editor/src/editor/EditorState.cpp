#include "EditorState.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/framework/IApplication.h"

#include "editor/panels/AssetInspector.h"
#include "editor/panels/ContentBrowser.h"
#include "editor/panels/FileSystemPanel.h"
#include "editor/panels/HierarchyPanel.h"
#include "editor/panels/LogPanel.h"
#include "editor/panels/MenuBar.h"
#include "editor/panels/PropertyPanel.h"
#include "editor/panels/RenderGraphViewer.h"
#include "editor/panels/RendererPanel.h"

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
#include "engine/private/ui/layout.h"
#include "engine/private/core/diagnostics/log_sink/CompositeLogger.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/view/ViewManager.h"

// #include "editor/edit/EditObjectCmd.h"
#include "editor/widgets/Image.h"

namespace cave {

using ecs::Entity;

EditorState::EditorState(IApplication& app)
    : AppState(app)
    , pie_(app)
    , debug_id_(MakeDebugId(this)) {

    AppServices& app_services = app.services();

    // services
    document_ = std::make_unique<DocumentService>(app_services,
                                                  services_);
    edit_ = std::make_unique<EditService>(app_services,
                                          services_);
    picking_ = std::make_unique<PickingService>(app_services,
                                                services_);
    thumbnail_ = std::make_unique<ThumbnailService>(app_services);
    icon_cache_ = std::make_unique<IconCache>(app_services.assetRegistry(),
                                              app_services.assetManager());

    selection_ = std::make_unique<SelectionService>(*this);
    shortcut_ = std::make_unique<ShortcutService>(*this);
    workspace_ = std::make_unique<Workspace>(*this);

    // panels
    content_browser_ = std::make_shared<ContentBrowser>(*this);
    menu_bar_ = std::make_shared<MenuBar>(*this);
    log_panel_ = std::make_shared<LogPanel>(*this);
    file_system_panel_ = std::make_shared<FileSystemPanel>(*this);
    asset_inspector_ = std::make_shared<AssetInspector>(*this,
                                                        services_);

    services_.document_ = document_.get();
    services_.edit_ = edit_.get();
    services_.icon_cache_ = icon_cache_.get();
    services_.picking_ = picking_.get();
    services_.selection_ = selection_.get();
    services_.shortcut_ = shortcut_.get();
    services_.thumbnail_ = thumbnail_.get();
    services_.workspace_ = workspace_.get();

    addPanel(log_panel_);
    addPanel(asset_inspector_);
    addPanel(std::make_shared<RendererPanel>(*this));
    addPanel(std::make_shared<HierarchyPanel>(*this));
    addPanel(std::make_shared<PropertyPanel>(*this));
    addPanel(content_browser_);
    // addPanel(std::make_shared<RenderGraphViewer>(*this));
    addPanel(file_system_panel_);
}

EditorState::~EditorState() {
    m_panels.clear();
}

void EditorState::onEnter(const StateRequest& request) {
    CAVE_PROFILE_EVENT();

    ImNodes::CreateContext();

    for (auto& panel : m_panels) {
        panel->onAttach();
    }

    SceneId edit_scene{};
    if (!request.arg1.empty()) {
        if (auto handle = app_.services().assetRegistry().FindByPath(request.arg1); handle.is_some()) {
            AssetHandle handle_ = handle.unwrap_unchecked();
            DocId doc_id = document_->openDoc({ handle_.GetGuid(), handle_.GetMeta()->type });
            if (IDocument* doc = document_->resolve(doc_id)) {
                edit_scene = doc->previewScene();
            }
        }
    }

    // load pie
    PIEStartDesc desc{};
    desc.game_id = request.arg0;
    desc.game_dll = std::format("{}_Debug.dll", desc.game_id);
    desc.edit_scene = edit_scene;

    pie_.start(std::move(desc));
}

void EditorState::onExit() {
    CAVE_PROFILE_EVENT();

    if (IsPlaying()) {
        LOG_INFO("@TODO: stop game module");
    }

    ImNodes::DestroyContext();

    pie_.stop();
}

void EditorState::tick(const FrameTime& p_time) {
    CAVE_PROFILE_EVENT();

    BusyInfo info;
    thumbnail_->Tick(p_time, info);

    if (IsPlaying()) {
        pie_.tick(p_time);
    }

    ImguiManager* imgui_manager = app_.GetImguiManager();
    DEV_ASSERT(imgui_manager);

    // @TODO: refactor this
    imgui_manager->BeginFrame();

    dockSpace();
    for (auto& panel : m_panels) {
        panel->drawUI();
    }

    workspace_->tick();

    ImGui::Render();

    commitModeSwitch();
}

void EditorState::RequestModeSwitch() {
    switch_mode_requested_ = true;
}

void EditorState::commitModeSwitch() {
    if (!switch_mode_requested_) {
        return;
    }

    const EditorState::Mode old_mode = mode_;
    mode_ = flipMode(mode_);

    switch (old_mode) {
        case EditorState::Mode::Editing: {
            PreviewScene preview = workspace_->focusedPreviewScene();
            pie_.onSimBegin(preview.scene_id, preview.view_id);
        } break;
        case EditorState::Mode::Playing: {
            pie_.onSimEnd();
        } break;
    }

#if USING(USE_LOG)
    constexpr const char* names[2] = { "Editing", "PIE" };
    LOG_INFO(LogChannel::Editor, "State {} -> {}",
             names[std::to_underlying(old_mode)],
             names[std::to_underlying(mode_)]);
#endif

    switch_mode_requested_ = false;
}

void EditorState::addPanel(std::shared_ptr<IEditorItem> p_panel) {
    m_panels.emplace_back(std::move(p_panel));
}

void EditorState::dockSpace() {
    CAVE_PROFILE_EVENT();

    ui::DockSpace({
        "DockSpace Demo",
        [this]() { menu_bar_->drawUI(); },
        [this]() {
            CompositeLogger& logger = CompositeLogger::singleton();
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
