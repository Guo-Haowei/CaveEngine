#pragma once
#include <cave/runtime/framework/IApplication.h>

// @TODO: check if all the includes are necessary
#include <engine/private/runtime/framework/AppState.h>
#include <engine/private/assets/asset_handle.h>
#include <engine/private/runtime/scene/scene.h>
#include <engine/private/runtime/scene/SceneComponent.h>

#include "editor/EditorWindow.h"
#include "editor/services/ShortcutDesc.h"
#include "editor/viewer/ViewerTab.h"

// @TODO: refactor this
#include "engine/private/runtime/framework/GameModuleLoader.h"

namespace cave {

enum class HandleInput : uint8_t;
enum class Key : uint16_t;

// pannels
class AssetInspector;
class FileSystemPanel;
class LogPanel;
class MenuBar;
class Viewer;

// services
class EditService;
class ShortcutService;
class RuntimeHost;

struct EditorContext {
    std::shared_ptr<ImageAsset> checkerboard;
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
    Viewer& GetViewer() { return *m_viewer.get(); }

    RuntimeHost& GetRuntimeHost() { return *m_runtime_host; }
    EditService& GetEditService() { return *m_edit_service; }
    ShortcutService& GetShortcutService() { return *m_shortcut_service; }

private:
    static Mode FlipState(Mode p_state) { return static_cast<Mode>(1 - std::to_underlying(p_state)); }
    void CommitModeSwitch();

    Mode m_state{ Mode::Editing };
    bool m_switch_mode_requested{ false };

    std::unique_ptr<RuntimeHost> m_runtime_host;
    std::unique_ptr<EditService> m_edit_service;
    std::unique_ptr<ShortcutService> m_shortcut_service;

    std::shared_ptr<AssetInspector> m_asset_inspector;
    std::shared_ptr<FileSystemPanel> m_file_system_panel;
    std::shared_ptr<LogPanel> m_log_panel;
    std::shared_ptr<MenuBar> m_menu_bar;
    std::shared_ptr<Viewer> m_viewer;

    std::vector<std::shared_ptr<EditorItem>> m_panels;

    // @TODO: refactor the following
    LoadedGameModule m_module{};

public:
    // @TODO: refactor this smelly context
    EditorContext context;

    void SetSelectedAsset(AssetHandle&& p_asset_handle) {
        m_selected_asset = std::move(p_asset_handle);
    }

    const AssetHandle& GetSelectedAsset() const { return m_selected_asset; }

private:
    void DockSpace();
    void AddPanel(std::shared_ptr<EditorItem> p_panel);

    AssetHandle m_selected_asset;
};

}  // namespace cave
