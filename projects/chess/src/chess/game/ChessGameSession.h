#pragma once
#include <array>
#include <memory>

#include "chess/agents/IPlayerAgent.h"

// clang-format off
namespace cave { class IHostServices; }
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
    explicit ChessGameSession(cave::IHostServices& host) noexcept;
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

    cave::IHostServices& host_;
    SessionPhase phase_{ SessionPhase::AwaitPlayerInput };

    std::unique_ptr<ChessMatchAuthority> auth_;
    std::unique_ptr<ChessGameClient> client_;

    std::unique_ptr<cave::GridSelectController> selector_;
    std::unique_ptr<ChessGridSelectorAdapter> grid_adapter_;

    std::array<std::unique_ptr<IPlayerAgent>, 2> agents_;
};

}  // namespace chess
