#pragma once
#include "cave/runtime/core/NonCopyable.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/gameplay/GameModeFactory.h"

// @TODO: refactor
#include "engine/private/runtime/core/os/timer.h"
#include "engine/private/runtime/framework/AppState.h"
#include "engine/private/runtime/framework/BootLoadPipeline.h"
#include "engine/private/runtime/framework/EventQueue.h"
#include "engine/private/runtime/framework/VFS.h"

namespace cave {

class Module;

class Application : public IApplication {
public:
    Application(const AppSpec& p_spec, AppType p_type);

    AppStateId GetStateId() const override;

    Result<void> Initialize() override;
    void Finalize() override;

    void RequestProject(std::string_view p_path) override;

    BootLoadPipeline& GetBootLoadPipeline() override;
    VFS& GetVFS() override { return m_vfs; }
    EventQueue& GetEventQueue() override { return m_event_queue; }
    GameModeFactory& GetGameModeFactory() override { return m_game_mode_factory; }

    AppType GetType() const override { return m_type; }

protected:
    [[nodiscard]] auto SetupModules() -> Result<void>;

    bool MainLoop() override;

    float UpdateTime();

    // @TODO: add CreateXXXManager for all managers
    virtual Result<ImguiManager*> CreateImguiManager();

    void RegisterModule(Module* p_module);

    const AppType m_type;

    AppStateMachine m_state_machine;
    GameModeFactory m_game_mode_factory;

    Timer m_timer;
    VFS m_vfs;

    EventQueue m_event_queue;
    std::vector<Module*> m_modules;

    std::unique_ptr<BootLoadPipeline> m_boot_load_pipeline;
};

}  // namespace cave
