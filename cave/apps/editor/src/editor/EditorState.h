#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/core/ids/SceneId.h"

#include "editor/document/DocId.h"

// @TODO: check if all the includes are necessary
#include <engine/private/runtime/framework/AppState.h>
#include <engine/private/assets/asset_handle.h>

#include "editor/panels/EditorWindow.h"
#include "editor/services/ShortcutDesc.h"

// @TODO: refactor this
#include "engine/private/runtime/framework/GameModuleLoader.h"

namespace cave {

class RuntimeHost;

// pannels
class AssetInspector;
class FileSystemPanel;
class LogPanel;
class MenuBar;

// services
class DocumentService;
class EditService;
class PickingService;
class SelectionService;
class ShortcutService;
class Workspace;

struct EditorContext {
    std::shared_ptr<ImageAsset> checkerboard;
};

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
    void Tick(float p_timestep) final;

    Option<StateRequest> PopRequest() final { return None(); }

    void RequestModeSwitch();
    bool IsPlaying() const { return m_state == Mode::Playing; }

#if USING(DEBUG_BUILD)
    const char* GetDebugName() final { return "EditorState"; }
#endif

    AssetInspector& GetAssetInspector() { return *m_asset_inspector.get(); }
    FileSystemPanel& GetFileSystemPanel() { return *m_file_system_panel.get(); }
    LogPanel& GetLogPanel() { return *m_log_panel.get(); }

    RuntimeHost& GetRuntimeHost() { return *m_runtime_host; }

    DocumentService& DocumentService() { return *m_document_service; }
    EditService& EditService() { return *m_edit_service; }
    PickingService& PickingService() { return *m_picking_service; }
    SelectionService& SelectionService() { return *m_selection_service; }
    ShortcutService& ShortcutService() { return *m_shortcut_service; }
    Workspace& Workspace() { return *m_workspace; }

    FocusedPreviewScene GetFocusedPreviewScene();

    // @TODO: move it to utility
    void OpenAddEntityPopup(ecs::Entity p_parent);

private:
    static Mode FlipState(Mode p_state) { return static_cast<Mode>(1 - std::to_underlying(p_state)); }
    void CommitModeSwitch();

    Mode m_state{ Mode::Editing };
    bool m_switch_mode_requested{ false };

    std::unique_ptr<RuntimeHost> m_runtime_host;

    std::unique_ptr<cave::DocumentService> m_document_service;
    std::unique_ptr<cave::EditService> m_edit_service;
    std::unique_ptr<cave::PickingService> m_picking_service;
    std::unique_ptr<cave::SelectionService> m_selection_service;
    std::unique_ptr<cave::ShortcutService> m_shortcut_service;
    std::shared_ptr<cave::Workspace> m_workspace;

    std::shared_ptr<AssetInspector> m_asset_inspector;
    std::shared_ptr<FileSystemPanel> m_file_system_panel;
    std::shared_ptr<LogPanel> m_log_panel;
    std::shared_ptr<MenuBar> m_menu_bar;

    std::vector<std::shared_ptr<IEditorItem>> m_panels;

    // @TODO: refactor the following
    LoadedGameModule m_module{};

public:
    // @TODO: refactor this smelly context
    EditorContext context;

private:
    void DockSpace();
    void AddPanel(std::shared_ptr<IEditorItem> p_panel);
};

}  // namespace cave
