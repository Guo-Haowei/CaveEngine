#pragma once
#include "engine/runtime/app_state.h"

namespace cave {

class ProjectBrowserState : public AppState {
public:
    ProjectBrowserState(Application& p_app);

    void OnEnter(const StateRequest& p_args) final;

    void OnExit() final;

    void Tick(float p_timestep) final;

    Option<StateRequest> PopRequest() final;

#if USING(DEBUG_BUILD)
    const char* GetDebugName() final { return "ProjectBrowser"; }
#endif
};

}  // namespace cave
