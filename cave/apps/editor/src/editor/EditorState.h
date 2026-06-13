#pragma once
#include "cave/core/ids/SceneId.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/framework/AppState.h"

#include "editor/document/DocId.h"
#include "editor/EditorServices.h"
#include "editor/play/PIESession.h"

namespace cave {

class IEditorItem;

// pannels
class AssetInspector;
class ContentBrowser;
class FileSystemPanel;
class LogPanel;
class MenuBar;

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

    // @TODO: dependency injection?
    ContentBrowser& GetAssetInspector() { return *content_browser_.get(); }
    FileSystemPanel& GetFileSystemPanel() { return *file_system_panel_.get(); }
    LogPanel& GetLogPanel() { return *log_panel_.get(); }

    EditorServices& services() { return services_; }

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
    std::unique_ptr<cave::DocumentService> document_;
    std::unique_ptr<cave::EditService> edit_;
    std::unique_ptr<cave::IconCache> icon_cache_;
    std::unique_ptr<cave::PickingService> picking_;
    std::unique_ptr<cave::SelectionService> selection_;
    std::unique_ptr<cave::ShortcutService> shortcut_;
    std::unique_ptr<cave::Workspace> workspace_;
    std::unique_ptr<cave::ThumbnailService> thumbnail_;

    // @TODO: use unique_ptr
    std::shared_ptr<AssetInspector> asset_inspector_;
    std::shared_ptr<ContentBrowser> content_browser_;
    std::shared_ptr<FileSystemPanel> file_system_panel_;
    std::shared_ptr<LogPanel> log_panel_;
    std::shared_ptr<MenuBar> menu_bar_;

    EditorServices services_;
    std::vector<std::shared_ptr<IEditorItem>> m_panels;
    const DebugId debug_id_;
};

}  // namespace cave
