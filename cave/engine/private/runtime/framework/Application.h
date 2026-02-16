#pragma once
#include "cave/core/NonCopyable.h"
#include "cave/core/time/Stopwatch.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/framework/AppState.h"
#include "engine/private/runtime/framework/BootLoadPipeline.h"
#include "engine/private/runtime/framework/EventQueue.h"
#include "engine/private/runtime/framework/VFS.h"
#include "engine/private/runtime/scene/SceneScheduler.h"

namespace cave {

class IService;

// @TODO: make this an impl class instead of virtual
class Application : public IApplication {
public:
    Application(const AppSpec& p_spec, AppType p_type);
    ~Application();

    AppStateId GetStateId() const override;

    Result<void> Initialize() override;
    void Finalize() override;

    QuitVote OnQuitRequested(const QuitContext&) override { return QuitVote::Allow; }

    void RequestProject(std::string_view p_path) override;

    BootLoadPipeline& GetBootLoadPipeline() override;
    VFS& GetVFS() override { return m_vfs; }
    EventQueue& GetEventQueue() override { return m_event_queue; }
    SceneScheduler& GetSceneScheduler() override { return *m_scene_scheduler; }

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
    VFS m_vfs;

    EventQueue m_event_queue;
    std::vector<IService*> m_modules;

    std::unique_ptr<BootLoadPipeline> m_boot_load_pipeline;
    std::unique_ptr<SceneScheduler> m_scene_scheduler;
};

}  // namespace cave
