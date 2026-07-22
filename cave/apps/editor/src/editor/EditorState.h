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
class AssetWorkspace;
class ContentBrowser;
class FileSystemPanel;
class MenuBar;
class PIESession;

class EditorState final : public AppState {
    enum class Mode : uint8_t {
        Editing = 0,
        Playing,
    };

public:
    EditorState(IApplication& app);
    ~EditorState();

    void onEnter(const StateRequest& args) override;
    void onExit() override;
    void tick(const FrameTime& time) override;

    Option<StateRequest> popRequest() override { return None(); }

    void requestModeSwitch();
    bool isPlaying() const { return m_mode == Mode::Playing; }

#if USING(DEBUG_BUILD)
    DebugId debugId() const override { return m_debug_id; }
#endif

    // @TODO: dependency injection?
    AssetWorkspace& assetInspector() { return *m_asset_workspace; }

    EditorServices& services() { return m_editor_services; }

    PIESession& PIE() { return *m_pie; }

private:
    void dockSpace();
    void addPanel(Ref<IEditorItem> panel);

    static Mode flipMode(Mode mode) { return static_cast<Mode>(1 - std::to_underlying(mode)); }
    void commitModeSwitch();

    // @TODO: refactor
    bool ensureGameModuleLoaded(const char* path);

    Mode m_mode{ Mode::Editing };
    bool m_switch_mode_requested{ false };

    Owner<PIESession> m_pie;

    Owner<DocumentService> m_document;
    Owner<DragDropService> m_drag_drop;
    Owner<EditService> m_edit;
    Owner<IconCache> m_icon_cache;
    Owner<PickingService> m_picking;
    Owner<SelectionService> m_selection;
    Owner<ShortcutService> m_shortcut;
    Owner<Workspace> m_workspace;
    Owner<ThumbnailService> m_thumbnail;

    // @TODO: use unique_ptr
    Ref<AssetWorkspace> m_asset_workspace;
    Ref<ContentBrowser> m_content_browser;
    Ref<FileSystemPanel> m_file_system_panel;
    Ref<MenuBar> m_menu_bar;

    EditorServices m_editor_services;
    Vector<Ref<IEditorItem>> m_panels;
    const DebugId m_debug_id;
};

}  // namespace cave
