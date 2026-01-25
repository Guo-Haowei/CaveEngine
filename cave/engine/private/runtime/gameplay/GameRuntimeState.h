// =============================================================================
// File: engine/private/runtime/gameplay/GameRuntimeState.h
// =============================================================================
#pragma once
#include "cave/runtime/gameplay/GameModeFactory.h"

#include "engine/private/runtime/framework/AppState.h"
#include "engine/private/runtime/framework/GameModuleLoader.h"

namespace cave {

class GameSession;

class GameRuntimeState : public AppState {
public:
    GameRuntimeState(IApplication& p_app);

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

    std::unique_ptr<GameSession> m_session;
};

}  // namespace cave
