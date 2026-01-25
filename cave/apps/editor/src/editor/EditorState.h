#pragma once
#include <cave/runtime/framework/IApplication.h>

// @TODO: check if all the includes are necessary
#include <engine/private/runtime/framework/AppState.h>
#include <engine/private/assets/asset_handle.h>
#include <engine/private/scene/scene.h>
#include <engine/private/scene/scene_component.h>

#include "editor/EditorWindow.h"
#include "editor/ShortcutManager.h"
#include "editor/viewer/ViewerTab.h"

namespace cave {

enum class HandleInput : uint8_t;
enum class Key : uint16_t;
class AssetInspector;
class EditorCommandBase;
class FileSystemPanel;
class LogPanel;
class MenuBar;
class Viewer;

struct EditorContext {
    float timestep{ 0 };
    std::shared_ptr<ImageAsset> checkerboard;
};

class EditorState final : public AppState {
public:
    EditorState(IApplication& p_app);

    void OnEnter(const StateRequest& p_args) final;
    void OnExit() final;
    void Tick(float p_timestep) final;

    Option<StateRequest> PopRequest() final;

    void RequestGamePlay();

#if USING(DEBUG_BUILD)
    const char* GetDebugName() final { return "EditorState"; }
#endif

    const auto& GetShortcuts() const {
        return m_shortcut_manager.GetShortcuts();
    }

private:
    Option<StateRequest> m_request;

    ShortcutManager m_shortcut_manager;

    // @TODO: refactor the following
public:
    void BufferCommand(std::shared_ptr<EditorCommandBase>&& p_command);
    void CommandInspectAsset(const Guid& p_guid);
    void CommandAddComponent(ComponentName p_type, ecs::Entity p_target);
    void CommandAddEntity(EntityType p_type, ecs::Entity p_parent);
    void CommandRemoveEntity(ecs::Entity p_target);
    void CommandDuplicateEntity(ecs::Entity p_target);

    // @TODO: refactor
    EditorContext context;

    void SetSelectedAsset(AssetHandle&& p_asset_handle) {
        m_selected_asset = std::move(p_asset_handle);
    }

    const AssetHandle& GetSelectedAsset() const { return m_selected_asset; }

    AssetInspector& GetAssetInspector() { return *m_asset_inspector.get(); }
    LogPanel& GetLogPanel() { return *m_log_panel.get(); }
    Viewer& GetViewer() { return *m_viewer.get(); }
    FileSystemPanel& GetFileSystemPanel() { return *m_file_system_panel.get(); }

private:
    void DockSpace();
    void AddPanel(std::shared_ptr<EditorItem> p_panel);

    void FlushCommand(Scene* p_scene);

    std::shared_ptr<AssetInspector> m_asset_inspector;
    std::shared_ptr<FileSystemPanel> m_file_system_panel;
    std::shared_ptr<LogPanel> m_log_panel;
    std::shared_ptr<MenuBar> m_menu_bar;
    std::shared_ptr<Viewer> m_viewer;

    std::vector<std::shared_ptr<EditorItem>> m_panels;

    std::list<std::shared_ptr<EditorCommandBase>> m_command_buffer;

    AssetHandle m_selected_asset;
};

}  // namespace cave
