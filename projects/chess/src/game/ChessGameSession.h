#pragma once
#include <array>
#include <memory>
#include "IPlayerAgent.h"

// clang-format off
namespace cave { class IHostServices; }
namespace cave { class GridSelectController; }
// clang-format on

namespace chess {

class ChessGameClient;
class ChessGridSelectorAdapter;
class ChessMatchAuthority;

#define SESSION_STATE_LIST          \
    SESSION_STATE(AwaitPlayerInput) \
    SESSION_STATE(ResolvingMove)    \
    SESSION_STATE(Animating)        \
    SESSION_STATE(GameOver)

enum class SessionState : uint8_t {
#define SESSION_STATE(Enum) Enum,
    SESSION_STATE_LIST
#undef SESSION_STATE
        Count,
};

enum class SessionMode : uint8_t {
    Local,
};

enum class PlayerKind : uint8_t {
    LocalHuman,
    LocalAI,
    RemoteNetwork,
};

struct SeatConfig {
    PlayerKind kind = PlayerKind::LocalHuman;
};

struct MatchConfig {
    SessionMode mode = SessionMode::Local;
    SeatConfig white{};
    SeatConfig black{};
};

class ChessGameSession {
public:
    explicit ChessGameSession(cave::IHostServices& p_host) noexcept;
    ~ChessGameSession();

    void Tick();

    void OnEnterBoot();

    void SetState(SessionState p_state);

private:
    std::unique_ptr<IPlayerAgent> CreatePlayer(PlayerId p_id,
                                               PlayerKind p_kind);

    void TickAwaitPlayerInput();
    void TickResolvingMove();
    void TickAnimating();
    void TickGameOver();

    bool IsAnimating() const;

    cave::IHostServices& m_host;
    SessionState m_state;

    std::unique_ptr<ChessMatchAuthority> m_auth;
    std::unique_ptr<ChessGameClient> m_client;

    std::unique_ptr<cave::GridSelectController> m_selector;
    std::unique_ptr<ChessGridSelectorAdapter> m_grid_adapter;

    std::array<std::unique_ptr<IPlayerAgent>, 2> m_agents;
};

}  // namespace chess
