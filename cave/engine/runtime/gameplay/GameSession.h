// =============================================================================
// File: engine/runtime/gameplay/GameSession.h
// =============================================================================
#pragma once
#include "IGameMode.h"
#include "GameModeFactory.h"

namespace cave {

class GameSession {
public:
    explicit GameSession(GameModeFactory& p_factory);

    bool Start(std::string_view p_mode_id);

    bool RequestSwitch(std::string_view p_mode_id);

    bool CommitModeSwitch();

    void Stop();

    void Tick(const GameFrameTime& p_time);

    IGameMode* GetMode() { return m_active.get(); }
    const IGameMode* GetMode() const { return m_active.get(); }

    std::string_view GetActiveModeId() const { return m_active_id; }

private:
    GameModeFactory& m_factory;

    std::unique_ptr<IGameMode> m_active;
    std::unique_ptr<IGameMode> m_pending;

    std::string m_active_id;
    std::string m_pending_id;
};

}  // namespace cave