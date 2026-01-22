#pragma once
#include "engine/runtime/app_state.h"

namespace cave {

class RuntimeState : public AppState {
public:
    RuntimeState(Application& p_app);

    void OnEnter(const StateRequest& p_args) final;

    void OnExit() final;

    void Tick(float p_timestep) final;

    Option<StateRequest> PopRequest() final;

#if USING(DEBUG_BUILD)
    const char* GetDebugName() final { return "RuntimeState"; }
#endif

private:
    Option<StateRequest> m_request{};
};

}  // namespace cave
