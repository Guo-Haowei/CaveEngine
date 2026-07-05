#pragma once
#include <array>
#include <memory>

#include "chess/agents/IPlayerAgent.h"

// clang-format off
namespace cave { struct SceneContext; }
namespace cave { class GridSelectController; }
// clang-format on

namespace chess {

class ChessGameClient;
class ChessGridSelectorAdapter;
class ChessMatchAuthority;

#define SESSION_PHASE_LIST          \
    SESSION_PHASE(AwaitPlayerInput) \
    SESSION_PHASE(ResolvingMove)    \
    SESSION_PHASE(Animating)        \
    SESSION_PHASE(GameOver)

enum class SessionPhase : uint8_t {
#define SESSION_PHASE(Enum) Enum,
    SESSION_PHASE_LIST
#undef SESSION_PHASE
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
    explicit ChessGameSession() noexcept;
    ~ChessGameSession();

    void tick(cave::SceneContext& ctx);

    void onEnterBoot(cave::SceneContext& ctx);

    void setPhase(SessionPhase phase);

private:
    auto createPlayer(core::Color side, PlayerKind kind) -> std::unique_ptr<IPlayerAgent>;

    void tickAwaitPlayerInput(cave::SceneContext& ctx);
    void tickResolvingMove(cave::SceneContext& ctx);
    void tickAnimating(cave::SceneContext& ctx);
    void tickGameOver(cave::SceneContext& ctx);

    bool isAnimating(cave::SceneContext& ctx) const;

    SessionPhase m_phase{ SessionPhase::AwaitPlayerInput };

    std::unique_ptr<ChessMatchAuthority> m_auth;
    std::unique_ptr<ChessGameClient> m_client;

    std::unique_ptr<cave::GridSelectController> m_selector;
    std::unique_ptr<ChessGridSelectorAdapter> m_grid_adapter;

    std::array<std::unique_ptr<IPlayerAgent>, 2> m_agents;
};

}  // namespace chess
