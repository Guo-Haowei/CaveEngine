#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/input/input_router.h"
#include "engine/runtime/application.h"
#include "engine/runtime/layer.h"
#include "engine/scene/scene.h"
#include "engine/scene/scene_component.h"
#include "editor/editor_window.h"
#include "editor/viewer/viewer_tab.h"

namespace cave {

enum class HandleInput : uint8_t;
enum class KeyCode : uint16_t;
class AssetInspector;
class EditorCommandBase;
class FileSystemPanel;
struct FolderTreeNode;
class LogPanel;
class MenuBar;
class Viewer;

enum {
    SHORT_CUT_SAVE_AS = 0,
    SHORT_CUT_SAVE,
    SHORT_CUT_OPEN,
    SHORT_CUT_UNDO,
    SHORT_CUT_REDO,
    SHORT_CUT_MAX,
};

struct EditorContext {
    float timestep{ 0 };
    std::shared_ptr<ImageAsset> checkerboard;
};

class EditorLayer : public Layer, public IInputHandler {
public:
    EditorLayer();
    virtual ~EditorLayer() = default;

    void OnAttach() override;
    void OnDetach() override;

    void OnUpdate(float p_timestep) override;
    void OnImGuiRender() override;

    void BufferCommand(std::shared_ptr<EditorCommandBase>&& p_command);
    void CommandInspectAsset(const Guid& p_guid);
    void CommandAddComponent(ComponentName p_type, ecs::Entity p_target);
    void CommandAddEntity(EntityType p_type, ecs::Entity p_parent);
    void CommandRemoveEntity(ecs::Entity p_target);

    HandleInputResult HandleInput(std::shared_ptr<InputEvent> p_input_event) override;

    const auto& GetShortcuts() const { return m_shortcuts; }

    CameraComponent* GetActiveCamera();

    EditorContext context;

    void SetSelectedAsset(AssetHandle&& p_asset_handle) {
        m_selected_asset = std::move(p_asset_handle);
    }

    const AssetHandle& GetSelectedAsset() const { return m_selected_asset; }

    AssetInspector& GetAssetInspector() { return *m_asset_inspector.get(); }
    LogPanel& GetLogPanel() { return *m_log_panel.get(); }
    Viewer& GetViewer() { return *m_viewer.get(); }
    FileSystemPanel& GetFileSystemPanel() { return *m_file_system_panel.get(); }

    const auto& GetAssetRoot() const { return m_asset_root; }

private:
    void DockSpace();
    void AddPanel(std::shared_ptr<EditorItem> p_panel);

    void FlushInputEvents();
    void FlushCommand(Scene* p_scene);

    std::shared_ptr<AssetInspector> m_asset_inspector;
    std::shared_ptr<FileSystemPanel> m_file_system_panel;
    std::shared_ptr<LogPanel> m_log_panel;
    std::shared_ptr<MenuBar> m_menu_bar;
    std::shared_ptr<Viewer> m_viewer;

    std::vector<std::shared_ptr<EditorItem>> m_panels;

    std::list<std::shared_ptr<EditorCommandBase>> m_command_buffer;

    struct ShortcutDesc {
        const char* name{ nullptr };
        const char* shortcut{ nullptr };
        std::function<void()> executeFunc{ nullptr };
        std::function<bool()> enabledFunc{ nullptr };

        KeyCode key{};
        bool ctrl{};
        bool alt{};
        bool shift{};
    };

    // @TODO: refactor shortcut
    std::array<ShortcutDesc, SHORT_CUT_MAX> m_shortcuts;

    AssetHandle m_selected_asset;

    std::vector<std::shared_ptr<InputEvent>> m_buffered_events;
    std::unique_ptr<FolderTreeNode> m_asset_root;
};

}  // namespace cave
