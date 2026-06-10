#pragma once
#include "cave/core/NonCopyable.h"
#include "cave/core/time/Stopwatch.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/intent/IntentDispatcher.h"

#include "engine/private/runtime/framework/AppState.h"
#include "engine/private/runtime/framework/EventQueue.h"
#include "engine/private/runtime/framework/VFS.h"
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

    AppStateId GetStateId() const override;

    Result<void> Initialize() override;
    void Finalize() override;

    QuitVote OnQuitRequested(const QuitContext&) override {
        return QuitVote::Allow;
    }

    EventQueue& GetEventQueue() override { return m_event_queue; }

    AppType GetType() const override { return m_type; }

protected:
    [[nodiscard]] auto SetupModules() -> Result<void>;

    bool MainLoop() override;

    float UpdateTime();

    // @TODO: add CreateXXXManager for all managers
    virtual Result<ImguiManager*> CreateImguiManager();

    void RegisterModule(IService* p_module);

    const AppType m_type;
    uint64_t m_frame_counter{};

    AppStateMachine m_state_machine;

    Stopwatch m_stopwatch;

    EventQueue m_event_queue;
    std::vector<IService*> m_modules;

    // @TODO: move above to AppServices
    IntentDispatcher intent_dispatcher_;
    VFS vfs_;
    SceneRegistry scene_registry_;

    std::unique_ptr<render::Renderer> renderer_;
    std::unique_ptr<ProjectManager> project_manager_;
    std::unique_ptr<SceneQueryService> scene_query_;
    std::unique_ptr<SceneScheduler> scene_scheduler_;
    std::unique_ptr<ViewManager> view_manager_;
    std::unique_ptr<UIRuntime> ui_;

    InputService* input_service_;
    TaskManager* task_manager_;
};

}  // namespace cave
