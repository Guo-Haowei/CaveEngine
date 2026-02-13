#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/SceneId.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/framework/AppState.h"

#include "editor/document/DocId.h"
#include "editor/play/PIESession.h"

namespace cave {

class IEditorItem;

// pannels
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

struct FocusedPreviewScene {
    DocId doc_id{};
    SceneId scene_id{};
    Scene* scene{ nullptr };
};

class EditorState final : public AppState {
    enum class Mode : uint8_t {
        Editing = 0,
        Playing,
    };

public:
    EditorState(IApplication& p_app);
    ~EditorState();

    void OnEnter(const StateRequest& p_args) final;
    void OnExit() final;
    void Tick(const FrameTime& p_time) final;

    Option<StateRequest> PopRequest() final { return None(); }

    void RequestModeSwitch();
    bool IsPlaying() const { return m_state == Mode::Playing; }

#if USING(DEBUG_BUILD)
    const char* GetDebugName() final { return "EditorState"; }
#endif

    ContentBrowser& GetAssetInspector() { return *m_content_browser.get(); }
    FileSystemPanel& GetFileSystemPanel() { return *m_file_system_panel.get(); }
    LogPanel& GetLogPanel() { return *m_log_panel.get(); }

    DocumentService& DocumentService() { return *m_document_service; }
    EditService& EditService() { return *m_edit_service; }
    IconCache& IconCache() { return *m_icon_cache; }
    PickingService& PickingService() { return *m_picking_service; }
    SelectionService& SelectionService() { return *m_selection_service; }
    ShortcutService& ShortcutService() { return *m_shortcut_service; }
    ThumbnailService& ThumbnailService() { return *m_thumbnail_service; }
    Workspace& Workspace() { return *m_workspace; }

    FocusedPreviewScene GetFocusedPreviewScene();

    // @TODO: move it to utility
    void OpenAddEntityPopup(ecs::Entity p_parent);

private:
    void DockSpace();
    void AddPanel(std::shared_ptr<IEditorItem> p_panel);

    static Mode FlipState(Mode p_state) { return static_cast<Mode>(1 - std::to_underlying(p_state)); }
    void CommitModeSwitch();

    Mode m_state{ Mode::Editing };
    bool m_switch_mode_requested{ false };

    PIESession m_pie;

    std::unique_ptr<cave::DocumentService> m_document_service;
    std::unique_ptr<cave::EditService> m_edit_service;
    std::unique_ptr<cave::IconCache> m_icon_cache;
    std::unique_ptr<cave::PickingService> m_picking_service;
    std::unique_ptr<cave::SelectionService> m_selection_service;
    std::unique_ptr<cave::ShortcutService> m_shortcut_service;
    std::unique_ptr<cave::Workspace> m_workspace;
    std::unique_ptr<cave::ThumbnailService> m_thumbnail_service;

    std::shared_ptr<ContentBrowser> m_content_browser;
    std::shared_ptr<FileSystemPanel> m_file_system_panel;
    std::shared_ptr<LogPanel> m_log_panel;
    std::shared_ptr<MenuBar> m_menu_bar;

    std::vector<std::shared_ptr<IEditorItem>> m_panels;
};

}  // namespace cave
