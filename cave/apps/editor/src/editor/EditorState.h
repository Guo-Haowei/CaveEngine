#pragma once
#include <cave/runtime/framework/IApplication.h>

// @TODO: check if all the includes are necessary
#include <engine/private/runtime/framework/AppState.h>
#include <engine/private/assets/asset_handle.h>
#include <engine/private/input/input_router.h>
#include <engine/private/scene/scene.h>
#include <engine/private/scene/scene_component.h>

#include "editor/EditorWindow.h"
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

enum {
    SHORT_CUT_SAVE_AS = 0,
    SHORT_CUT_SAVE,
    SHORT_CUT_OPEN,
    SHORT_CUT_UNDO,
    SHORT_CUT_REDO,
    SHORT_CUT_DEBUG,
    SHORT_CUT_MAX,
};

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

private:
    Option<StateRequest> m_request;

public:
    // @TODO: refactor the following
    void BufferCommand(std::shared_ptr<EditorCommandBase>&& p_command);
    void CommandInspectAsset(const Guid& p_guid);
    void CommandAddComponent(ComponentName p_type, ecs::Entity p_target);
    void CommandAddEntity(EntityType p_type, ecs::Entity p_parent);
    void CommandRemoveEntity(ecs::Entity p_target);
    void CommandDuplicateEntity(ecs::Entity p_target);

    const auto& GetShortcuts() const { return m_shortcuts; }

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

    // @TODO: refactor shortcut
    struct ShortcutDesc {
        const char* name{ nullptr };
        const char* shortcut{ nullptr };
        std::function<void()> executeFunc{ nullptr };
        std::function<bool()> enabledFunc{ nullptr };

        Key key{};
        bool ctrl{};
        bool alt{};
        bool shift{};
    };

    // @TODO: refactor shortcut
    std::array<ShortcutDesc, SHORT_CUT_MAX> m_shortcuts;

    AssetHandle m_selected_asset;
};

}  // namespace cave
