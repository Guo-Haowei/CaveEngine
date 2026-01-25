// =============================================================================
// File: engine/runtime/gameplay/GameSession.h
// =============================================================================
#pragma once
#include "engine/runtime/gameplay/IGameMode.h"
#include "engine/runtime/gameplay/IPlayerAgent.h"
#include "engine/runtime/gameplay/GameModeFactory.h"

namespace cave {

class IPlayerAgent;

class GameSession {
    using Player = std::unique_ptr<IPlayerAgent>;

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

    void AddPlayer(Player p_player) {
        m_players.emplace_back(std::move(p_player));
    }

    uint32_t PlayerCount() const {
        return static_cast<uint32_t>(m_players.size());
    }

    IPlayerAgent* GetPlayer(uint32_t p_index);

private:
    GameModeFactory& m_factory;

    std::unique_ptr<IGameMode> m_active;
    std::unique_ptr<IGameMode> m_pending;

    std::string m_active_id;
    std::string m_pending_id;

    std::vector<Player> m_players;
};

}  // namespace cave