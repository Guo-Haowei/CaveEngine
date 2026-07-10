#include "EditorState.h"

#include "cave/core/diagnostics/CompositeLogger.h"
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
#include "editor/panels/RendererPanel.h"

#include "editor/services/DocumentService.h"
#include "editor/services/EditService.h"
#include "editor/services/IconCache.h"
#include "editor/services/PickingService.h"
#include "editor/services/SelectionService.h"
#include "editor/services/ShortcutService.h"
#include "editor/services/ThumbnailService.h"
#include "editor/services/Workspace.h"

#include "editor/play/PIESession.h"
#include "editor/EditorAssetManager.h"

// @TODO: refactor
#include <imgui/imgui_internal.h>
#include <imnodes/imnodes.h>

#include "Enums.h"
#include "engine/private/core/os/os.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/view/ViewManager.h"
#include "engine/private/ui/layout.h"

#include "cave/game/GameModuleHandle.h"
#include "editor/widgets/Image.h"

namespace cave {

using ecs::Entity;

EditorState::EditorState(IApplication& app)
    : AppState(app)
    , m_pie(std::make_unique<PIESession>(app.services()))
    , m_debug_id(MakeDebugId(this)) {

    EngineServices& app_services = app.services();

    // services
    m_document = std::make_unique<DocumentService>(app_services, m_editor_services);
    m_editor_services.document_ = m_document.get();

    m_edit = std::make_unique<EditService>(app_services, m_editor_services);
    m_editor_services.edit_ = m_edit.get();

    m_picking = std::make_unique<PickingService>(app_services, m_editor_services);
    m_editor_services.picking_ = m_picking.get();

    m_thumbnail = std::make_unique<ThumbnailService>(app_services);
    m_editor_services.thumbnail_ = m_thumbnail.get();

    m_icon_cache = std::make_unique<IconCache>(app_services.assetRegistry(), app_services.assetManager());
    m_editor_services.icon_cache_ = m_icon_cache.get();

    m_selection = std::make_unique<SelectionService>();
    m_editor_services.selection_ = m_selection.get();

    m_shortcut = std::make_unique<ShortcutService>(*this);
    m_editor_services.shortcut_ = m_shortcut.get();

    m_workspace = std::make_unique<Workspace>(*this);
    m_editor_services.workspace_ = m_workspace.get();

    // panels
    m_content_browser = std::make_shared<ContentBrowser>(*this);
    m_menu_bar = std::make_shared<MenuBar>(*this);
    m_log_panel = std::make_shared<LogPanel>(*this);
    m_file_system_panel = std::make_shared<FileSystemPanel>(*this);
    m_asset_inspector = std::make_shared<AssetInspector>(*this);

    addPanel(m_log_panel);
    addPanel(m_asset_inspector);
    addPanel(std::make_shared<RendererPanel>(*this));
    addPanel(std::make_shared<HierarchyPanel>(*this));
    addPanel(std::make_shared<PropertyPanel>(*this));
    addPanel(m_content_browser);
    addPanel(m_file_system_panel);

    static_cast<EditorAssetManager&>(app_services.assetManager()).setEditorServices(&m_editor_services);
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

    // load pie
    auto game_dll = std::format("{}.dll", request.arg0);
    ensureGameModuleLoaded(game_dll.c_str());

    m_workspace->restoreTabs();
}

void EditorState::onExit() {
    CAVE_PROFILE_EVENT();

    if (isPlaying()) {
        LOG_INFO("@TODO: stop game module");
    }

    ImNodes::DestroyContext();

    for (auto& panel : m_panels) {
        panel->onDetach();
    }
}

void EditorState::tick(const FrameTime& time) {
    CAVE_PROFILE_EVENT();

    BusyInfo info;
    m_thumbnail->tick(time, info);

    if (isPlaying()) {
        m_pie->tick(time);
    }

    ImGuiService* imgui = m_app.services().imgui;
    DEV_ASSERT(imgui);

    // @TODO: refactor this
    imgui->beginFrame();

    dockSpace();
    for (auto& panel : m_panels) {
        panel->drawUI();
    }

    m_workspace->tick(time.dt);

    ImGui::Render();

    commitModeSwitch();
}

void EditorState::requestModeSwitch() {
    m_switch_mode_requested = true;
}

void EditorState::commitModeSwitch() {
    if (!m_switch_mode_requested) {
        return;
    }

    const EditorState::Mode old_mode = m_mode;

    bool ok = false;
    switch (old_mode) {
        case EditorState::Mode::Editing: {
            PreviewScene preview = m_workspace->focusedPreviewScene();
            ok = m_pie->beginPIESession(preview.guid, preview.view_id);
        } break;
        case EditorState::Mode::Playing: {
            ok = m_pie->endPIESession();
        } break;
    }

    if (ok) {
        m_mode = flipMode(m_mode);
    } else {
        LOG_ERROR(LogChannel::Asset, "failed to start PIE session");
    }

    m_switch_mode_requested = false;
}

void EditorState::addPanel(std::shared_ptr<IEditorItem> p_panel) {
    m_panels.emplace_back(std::move(p_panel));
}

void EditorState::dockSpace() {
    CAVE_PROFILE_EVENT();

    ui::DockSpace({
        "DockSpace Demo",
        [this]() { m_menu_bar->drawUI(); },
        [this]() {
            CompositeLogger& logger = OS::singleton().logger();
            const uint32_t error_count = static_cast<uint32_t>(logger.errorLogs().size());
            const uint32_t warning_count = static_cast<uint32_t>(logger.warningLogs().size());

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

// @TODO: refactor
bool EditorState::ensureGameModuleLoaded(const char* path) {
    auto& game_module = m_app.services().gameModule();
    if (!game_module.loadFromDll(path, m_app.services().nativeScripts())) {
        return false;
    }

    return game_module.get() != nullptr;
}

}  // namespace cave
