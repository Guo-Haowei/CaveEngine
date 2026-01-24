// =============================================================================
// File: engine/runtime/gameplay/IGameMode.h
// =============================================================================
#pragma once

namespace cave {

class GameSession;

struct GameFrameTime {
    float dt = 0.0f;
    uint64_t frameIndex = 0;
};

#if 0
// Optional: if you want a standardized way to expose "view state" to UI.
struct GameViewSnapshot {
    uint64_t type = 0;                 // game-defined snapshot type id
    std::vector<uint8_t> payload;      // game-defined bytes (or empty)
};
#endif

class IGameMode {
public:
    virtual ~IGameMode() = default;

    virtual std::string_view GetId() const = 0;

    virtual void OnEnter(GameSession& p_session) = 0;

    virtual void OnExit(GameSession& p_session) = 0;

    virtual void Tick(GameSession& p_session, const GameFrameTime& p_time) = 0;

#if 0
    // Optional: allow UI/editor to pull a lightweight snapshot (no raw pointers).
    virtual bool GetViewSnapshot(GameViewSnapshot& /*p_out*/) const { return false; }
#endif
};

}  // namespace cave
