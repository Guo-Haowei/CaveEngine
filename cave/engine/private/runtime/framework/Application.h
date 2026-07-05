#pragma once
#include "cave/core/base/NonCopyable.h"
#include "cave/core/time/Stopwatch.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/intent/IntentBus.h"
#include "cave/runtime/script/native/NativeScriptRegistry.h"

#include "engine/private/runtime/display/DebugDrawService.h"
#include "engine/private/runtime/framework/AppState.h"
#include "engine/private/runtime/framework/EventQueue.h"
#include "engine/private/runtime/framework/VFS.h"
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/scene/SceneQueryService.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/scene/SceneScheduler.h"
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

    // @TODO: add CreateXXXManager for all managers
    virtual Result<ImguiManager*> createImguiManager();

    void registerModule(IService* module);

    const AppType m_app_type;
    uint64_t m_frame_counter{};
    AppStateMachine m_state_machine;
    Stopwatch m_stopwatch;

    EventQueue m_event_queue;
    std::vector<IService*> m_subsystems;

    DebugDrawService m_debug_draw;
    IntentBus m_intent_bus;
    VFS m_vfs;
    SceneRegistry m_scene_registry;
    NativeScriptRegistry m_native_scripts;

    std::unique_ptr<ProjectManager> m_project_manager;
    std::unique_ptr<SceneQueryService> m_scene_query;
    std::unique_ptr<SceneScheduler> m_scene_scheduler;
    std::unique_ptr<ViewManager> m_view_manager;
    std::unique_ptr<UIRuntime> m_ui;
    std::unique_ptr<render::Renderer> m_renderer;

    // @TODO: do not use raw pointers
    DisplayService* m_display_service{};
    InputService* m_input_service{};
    GameInput m_game_input;
    TaskManager* m_task_manager{};
    AssetRegistry* m_asset_registry{};
    IAssetManager* m_asset_manager{};
    render::IRenderDevice* m_render_device{};
};

}  // namespace cave
