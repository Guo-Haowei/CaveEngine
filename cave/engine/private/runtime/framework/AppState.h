#pragma once
#include "cave/core/ids/DebugId.h"
#include "cave/core/time/FrameTime.h"

namespace cave {

class IApplication;

enum class AppStateId : uint8_t {
    ProjectBrowser = 0,
    LoadingScreen,
    Editor,
    GameRuntime,
    Count,
};

struct StateRequest {
    AppStateId next{ AppStateId::ProjectBrowser };
    std::string arg0;
};

class AppState {
public:
    AppState(IApplication& app)
        : app_(app) {}

    virtual ~AppState() = default;

    virtual void onEnter(const StateRequest& args) = 0;

    virtual void onExit() = 0;

    virtual void tick(const FrameTime& time) = 0;

    virtual Option<StateRequest> popRequest() = 0;

#if USING(DEBUG_BUILD)
    virtual DebugId debugId() const = 0;
#endif

    IApplication& app() { return app_; }

protected:
    IApplication& app_;
};

class AppStateMachine {
public:
    using CreateFunc = std::unique_ptr<AppState> (*)(IApplication&);

    AppStateMachine(IApplication& app)
        : app_(app) {}

    void initialize(AppStateId initial_state);

    void shutdown();

    void tick(const FrameTime& time);

    AppStateId stateId() const { return state_id_; }

    AppState* appState() const { return state_.get(); }

    static void registerCreateFunc(AppStateId state_id, CreateFunc func);

    static auto createState(IApplication& app, AppStateId state_id)
        -> std::unique_ptr<AppState>;

private:
    void switchTo(const StateRequest& request);

    inline static CreateFunc s_create_funcs[std::to_underlying(AppStateId::Count)];

    IApplication& app_;
    std::unique_ptr<AppState> state_;
    AppStateId state_id_;
};

}  // namespace cave
