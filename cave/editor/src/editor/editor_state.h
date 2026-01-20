#pragma once
#include "engine/runtime/app_state.h"

namespace cave {

class EditorState final : public AppState {
public:
    void OnEnter(const StateRequest& p_args) final;

    void Tick(float p_timestep) final;

    StateRequest PopRequest() final;

#if USING(DEBUG_BUILD)
    const char* GetDebugName() final { return "EditorState"; }
#endif

private:
};

}  // namespace cave
