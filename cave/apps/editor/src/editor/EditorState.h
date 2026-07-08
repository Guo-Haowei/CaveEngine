#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/SceneId.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/framework/AppState.h"

#include "editor/document/DocId.h"
#include "editor/services/EditorServices.h"

namespace cave {

class IEditorItem;

// pannels
class AssetInspector;
class ContentBrowser;
class FileSystemPanel;
class LogPanel;
class MenuBar;
class PIESession;

class EditorState final : public AppState {
    enum class Mode : uint8_t {
        Editing = 0,
        Playing,
    };

public:
    EditorState(IApplication& p_app);
    ~EditorState();

    void onEnter(const StateRequest& p_args) override;
    void onExit() override;
    void tick(const FrameTime& p_time) override;

    Option<StateRequest> popRequest() override { return None(); }

    void requestModeSwitch();
    bool isPlaying() const { return m_mode == Mode::Playing; }

#if USING(DEBUG_BUILD)
    DebugId debugId() const override { return m_debug_id; }
#endif

    // @TODO: dependency injection?
    AssetInspector& assetInspector() { return *m_asset_inspector; }

    EditorServices& services() { return m_editor_services; }

    PIESession& PIE() { return *m_pie; }

private:
    void dockSpace();
    void addPanel(std::shared_ptr<IEditorItem> panel);

    static Mode flipMode(Mode mode) { return static_cast<Mode>(1 - std::to_underlying(mode)); }
    void commitModeSwitch();

    // @TODO: refactor
    bool ensureGameModuleLoaded(const char* path);

    Mode m_mode{ Mode::Editing };
    bool m_switch_mode_requested{ false };

    std::unique_ptr<PIESession> m_pie;

    // @TODO: move to EditorServices
    std::unique_ptr<DocumentService> m_document;
    std::unique_ptr<EditService> m_edit;
    std::unique_ptr<IconCache> m_icon_cache;
    std::unique_ptr<PickingService> m_picking;
    std::unique_ptr<SelectionService> m_selection;
    std::unique_ptr<ShortcutService> m_shortcut;
    std::unique_ptr<Workspace> m_workspace;
    std::unique_ptr<ThumbnailService> m_thumbnail;

    // @TODO: use unique_ptr
    std::shared_ptr<AssetInspector> m_asset_inspector;
    std::shared_ptr<ContentBrowser> m_content_browser;
    std::shared_ptr<FileSystemPanel> m_file_system_panel;
    std::shared_ptr<LogPanel> m_log_panel;
    std::shared_ptr<MenuBar> m_menu_bar;

    EditorServices m_editor_services;
    std::vector<std::shared_ptr<IEditorItem>> m_panels;
    const DebugId m_debug_id;
};

}  // namespace cave
