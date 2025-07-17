#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/input/input_router.h"
#include "engine/runtime/application.h"
#include "engine/runtime/layer.h"
#include "engine/scene/scene.h"
#include "engine/scene/scene_component.h"
#include "editor/editor_window.h"
#include "editor/tools/tool.h"

namespace cave {

enum class HandleInput : uint8_t;
enum class KeyCode : uint16_t;
struct ImageAsset;
class EditorCommandBase;
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
};

class EditorLayer : public Layer, public IInputHandler {
public:
    EditorLayer();
    virtual ~EditorLayer() = default;

    void OnAttach() override;
    void OnDetach() override;

    void OnUpdate(float p_timestep) override;
    void OnImGuiRender() override;

    void SelectEntity(ecs::Entity p_selected);
    ecs::Entity GetSelectedEntity() const { return m_selected; }

    uint64_t GetDisplayedImage() const { return m_displayedImage; }
    void SetDisplayedImage(uint64_t p_image) { m_displayedImage = p_image; }

    void BufferCommand(std::shared_ptr<EditorCommandBase>&& p_command);
    void CommandInspectAsset(const Guid& p_guid);
    void CommandAddComponent(ComponentName p_type, ecs::Entity p_target);
    void CommandAddEntity(EntityType p_type, ecs::Entity p_parent);
    void CommandRemoveEntity(ecs::Entity p_target);

    HandleInputResult HandleInput(std::shared_ptr<InputEvent> p_input_event) override;

    const auto& GetShortcuts() const { return m_shortcuts; }

    CameraComponent& GetActiveCamera();

    EditorContext context;

    void OpenTool(ToolType p_type, const Guid& p_guid);
    ITool* GetActiveTool();

    void SetSelectedAsset(AssetHandle&& p_asset_handle) {
        m_selected_asset = std::move(p_asset_handle);
    }

    const AssetHandle& GetSelectedAsset() const { return m_selected_asset; }

private:
    void DockSpace(Scene* p_scene);
    void AddPanel(std::shared_ptr<EditorItem> p_panel);

    void FlushCommand(Scene* p_scene);

    std::shared_ptr<MenuBar> m_menuBar;
    std::shared_ptr<Viewer> m_viewer;

    std::vector<std::shared_ptr<EditorItem>> m_panels;
    ecs::Entity m_selected;

    uint64_t m_displayedImage = 0;
    std::list<std::shared_ptr<EditorCommandBase>> m_commandBuffer;

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

    std::array<std::unique_ptr<ITool>, std::to_underlying(ToolType::Count)> m_tools;
    ToolType m_current_tool{ ToolType ::None };
};

}  // namespace cave
