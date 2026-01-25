// =============================================================================
// File: engine/private/runtime/gameplay/GameRuntimeState.h
// =============================================================================
#pragma once
#include "engine/private/runtime/framework/AppState.h"
#include "engine/private/runtime/framework/GameModuleLoader.h"
#include "engine/private/runtime/gameplay/GameModeFactory.h"

namespace cave {

class GameRuntimeState : public AppState {
public:
    GameRuntimeState(Application& p_app);

    void OnEnter(const StateRequest& p_args) final;

    void OnExit() final;

    void Tick(float p_timestep) final;

    Option<StateRequest> PopRequest() final;

#if USING(DEBUG_BUILD)
    const char* GetDebugName() final { return "GameRuntimeState"; }
#endif

private:
    Option<StateRequest> m_request{};
    LoadedGameModule m_module{};
    GameModeFactory m_factory;
};

}  // namespace cave
