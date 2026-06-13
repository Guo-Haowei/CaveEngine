#pragma once
#include "cave/core/ids/SceneId.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/framework/AppState.h"

#include "editor/document/DocId.h"
#include "editor/play/PIESession.h"

namespace cave {

class IEditorItem;

// pannels
class AssetInspector;
class ContentBrowser;
class FileSystemPanel;
class LogPanel;
class MenuBar;

// services
class DocumentService;
class EditService;
class IconCache;
class PickingService;
class SelectionService;
class ShortcutService;
class ThumbnailService;
class Workspace;

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

    void RequestModeSwitch();
    bool IsPlaying() const { return mode_ == Mode::Playing; }

#if USING(DEBUG_BUILD)
    DebugId debugId() const override { return debug_id_; }
#endif

    ContentBrowser& GetAssetInspector() { return *content_browser_.get(); }
    FileSystemPanel& GetFileSystemPanel() { return *file_system_panel_.get(); }
    LogPanel& GetLogPanel() { return *log_panel_.get(); }

    DocumentService& DocumentService() { return *m_document_service; }
    EditService& EditService() { return *m_edit_service; }
    IconCache& IconCache() { return *m_icon_cache; }
    PickingService& PickingService() { return *m_picking_service; }
    SelectionService& SelectionService() { return *m_selection_service; }
    ShortcutService& ShortcutService() { return *m_shortcut_service; }
    ThumbnailService& ThumbnailService() { return *m_thumbnail_service; }
    Workspace& Workspace() { return *m_workspace; }
    PIESession& PIE() { return pie_; }

private:
    void dockSpace();
    void addPanel(std::shared_ptr<IEditorItem> panel);

    static Mode flipMode(Mode mode) { return static_cast<Mode>(1 - std::to_underlying(mode)); }
    void commitModeSwitch();

    Mode mode_{ Mode::Editing };
    bool switch_mode_requested_{ false };

    PIESession pie_;

    // @TODO: move to EditorServices
    std::unique_ptr<cave::DocumentService> m_document_service;
    std::unique_ptr<cave::EditService> m_edit_service;
    std::unique_ptr<cave::IconCache> m_icon_cache;
    std::unique_ptr<cave::PickingService> m_picking_service;
    std::unique_ptr<cave::SelectionService> m_selection_service;
    std::unique_ptr<cave::ShortcutService> m_shortcut_service;
    std::unique_ptr<cave::Workspace> m_workspace;
    std::unique_ptr<cave::ThumbnailService> m_thumbnail_service;

    // @TODO: use unique_ptr
    std::shared_ptr<AssetInspector> asset_inspector_;
    std::shared_ptr<ContentBrowser> content_browser_;
    std::shared_ptr<FileSystemPanel> file_system_panel_;
    std::shared_ptr<LogPanel> log_panel_;
    std::shared_ptr<MenuBar> menu_bar_;

    std::vector<std::shared_ptr<IEditorItem>> m_panels;
    const DebugId debug_id_;
};

}  // namespace cave
