#pragma once
#include "cave/core/base/NonCopyable.h"
#include "cave/core/time/Stopwatch.h"
#include "cave/runtime/game/GameModuleHandle.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/intent/IntentBus.h"
#include "cave/runtime/script/native/NativeScriptRegistry.h"

#include "engine/private/runtime/display/Canvas.h"
#include "engine/private/runtime/framework/AppState.h"
#include "engine/private/runtime/framework/EventQueue.h"
#include "engine/private/runtime/framework/VFS.h"
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/ui/UIRuntime.h"

namespace cave {

class IService;

// @TODO: make this an impl class instead of virtual
class Application : public IApplication {
public:
    Application(const AppSpec& spec, AppType type);
    ~Application();

    AppStateId stateId() const override;

    Result<void> initialize() override;
    void finalize() override;

    QuitVote onQuitRequested(const QuitContext&) override {
        return QuitVote::Allow;
    }

    EventQueue& eventQueue() override { return m_event_queue; }

    AppType appType() const override { return m_app_type; }

protected:
    [[nodiscard]] auto setupModules() -> Result<void>;

    bool mainLoop() override;

    float updateTime();

    void registerModule(IService* module);

    const AppType m_app_type;
    uint64_t m_frame_counter{};
    AppStateMachine m_state_machine;
    Stopwatch m_stopwatch;

    EventQueue m_event_queue;
    Vector<IService*> m_subsystems;

    Canvas m_canvas;
    IntentBus m_intent_bus;
    VFS m_vfs;
    NativeScriptRegistry m_native_scripts;
    // @TODO: module manager?
    GameModuleHandle m_game_module_handle;

    Owner<ProjectManager> m_project_manager;
    Owner<SceneRegistry> m_scene_registry;
    Owner<SceneScheduler> m_scene_scheduler;
    Owner<ViewManager> m_view_manager;
    Owner<UIRuntime> m_ui;
    Owner<render::Renderer> m_renderer;

    // @TODO: do not use raw pointers
    ImGuiService* m_imgui{};
    DisplayService* m_display_service{};
    InputService* m_input_service{};
    GameInput m_game_input;
    TaskManager* m_task_manager{};
    AssetRegistry* m_asset_registry{};
    IAssetManager* m_asset_manager{};
    render::IRenderDevice* m_render_device{};
};

}  // namespace cave
