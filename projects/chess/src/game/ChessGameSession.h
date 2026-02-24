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

enum class SessionState : uint8_t {
    Boot,
    Playing,
    GameOver,
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
    explicit ChessGameSession() noexcept;
    ~ChessGameSession();

    void Tick(cave::IHostServices& p_host);

private:
    std::unique_ptr<IPlayerAgent> CreatePlayer(PlayerId p_id,
                                               PlayerKind p_kind);

    void Cleanup();
    void EnterBoot(cave::IHostServices& p_host);

    void TickBoot(cave::IHostServices& p_host);
    void TickPlaying(cave::IHostServices& p_host);
    void TickGameOver(cave::IHostServices& p_host);

    SessionState m_state = SessionState::Boot;

    std::unique_ptr<ChessMatchAuthority> m_auth;
    std::unique_ptr<ChessGameClient> m_client;

    std::unique_ptr<cave::GridSelectController> m_selector;
    std::unique_ptr<ChessGridSelectorAdapter> m_grid_adapter;

    std::array<std::unique_ptr<IPlayerAgent>, 2> m_agents;
};

}  // namespace chess
