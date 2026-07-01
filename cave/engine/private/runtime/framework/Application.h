#pragma once
#include "cave/core/base/NonCopyable.h"
#include "cave/core/time/Stopwatch.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/intent/IntentDispatcher.h"
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

    EventQueue& eventQueue() override { return event_queue_; }

    AppType appType() const override { return type_; }

protected:
    [[nodiscard]] auto setupModules() -> Result<void>;

    bool mainLoop() override;

    float updateTime();

    // @TODO: add CreateXXXManager for all managers
    virtual Result<ImguiManager*> createImguiManager();

    void registerModule(IService* module);

    const AppType type_;
    uint64_t frame_counter_{};
    AppStateMachine state_machine_;
    Stopwatch stopwatch_;

    EventQueue event_queue_;
    std::vector<IService*> subsystems_;

    DebugDrawService debug_draw_;
    IntentDispatcher intent_dispatcher_;
    VFS vfs_;
    SceneRegistry scene_registry_;
    NativeScriptRegistry native_scripts_;

    std::unique_ptr<ProjectManager> project_manager_;
    std::unique_ptr<SceneQueryService> scene_query_;
    std::unique_ptr<SceneScheduler> scene_scheduler_;
    std::unique_ptr<ViewManager> view_manager_;
    std::unique_ptr<UIRuntime> ui_;
    std::unique_ptr<render::Renderer> renderer_;

    // @TODO: do not use raw pointers
    DisplayService* display_service_{};
    InputService* input_service_{};
    GameInput game_input_;
    TaskManager* task_manager_{};
    AssetRegistry* asset_registry_{};
    IAssetManager* asset_manager_{};
    render::IRenderDevice* render_device_{};
};

}  // namespace cave
