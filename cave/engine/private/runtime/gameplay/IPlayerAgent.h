// =============================================================================
// File: engine/private/runtime/gameplay/IPlayerAgent.h
// =============================================================================
#pragma once

namespace cave {

struct GameDecision;
struct GameFrameTime;
class GameSession;
class IGameMode;

class IPlayerAgent {
public:
    virtual ~IPlayerAgent() = default;

    virtual void OnMatchStart(GameSession& p_session, const IGameMode& p_mode, int p_player_index) = 0;
    virtual void OnMatchEnd(GameSession& p_session, const IGameMode& p_mode, int p_player_index) = 0;

    virtual void Tick(GameSession& /*p_session*/,
                      const IGameMode& /*p_mode*/,
                      int /*p_player_idx*/,
                      const GameFrameTime& /*p_time*/) {}

    virtual bool PollDecision(GameSession& p_session,
                              const IGameMode& p_mode,
                              int p_player_idx,
                              GameDecision& p_out_decision) = 0;
};

}  // namespace cave
