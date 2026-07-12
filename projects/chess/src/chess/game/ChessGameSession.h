#pragma once
#include <array>

#include "cave/core/memory/Pointer.h"

#include "chess/agents/IPlayerAgent.h"

// clang-format off
namespace cave { class GridSelectController; }
namespace cave { class SceneRuntime; }
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
    explicit ChessGameSession(cave::SceneRuntime& runtime,
                              cave::IntentBus& intent_bus) noexcept;
    ~ChessGameSession();

    void tick();

    void onEnterBoot();

    void setPhase(SessionPhase phase);

private:
    auto createPlayer(core::Color side, PlayerKind kind) -> std::unique_ptr<IPlayerAgent>;

    void tickAwaitPlayerInput();
    void tickResolvingMove();
    void tickAnimating();
    void tickGameOver();

    bool isAnimating() const;

    cave::SceneRuntime& m_runtime;
    cave::IntentBus& m_intent_bus;

    SessionPhase m_phase{ SessionPhase::AwaitPlayerInput };

    cave::Owner<ChessMatchAuthority> m_auth;
    cave::Owner<ChessGameClient> m_client;

    cave::Owner<cave::GridSelectController> m_selector;
    cave::Owner<ChessGridSelectorAdapter> m_grid_adapter;

    std::array<std::unique_ptr<IPlayerAgent>, 2> m_agents;
};

}  // namespace chess
