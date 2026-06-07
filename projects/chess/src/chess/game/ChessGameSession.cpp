#include "ChessGameSession.h"

#include "cave/game/IHostServices.h"
#include "cave/runtime/controller/GridSelectController.h"
#include "cave/runtime/input/IGameInput.h"
#include "cave/runtime/intent/IntentDispatcher.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneQuery.h"

#include "chess/agents/ChessAIAgent.h"
#include "chess/agents/LocalHumanAgent.h"
#include "chess/game/ChessGameClient.h"
#include "chess/game/ChessIntent.h"
#include "chess/game/ChessMatchAuthority.h"
#include "chess/presentation/ChessGridSelectorAdapter.h"
#include "chess/states/GameOverState.h"

namespace chess {

using namespace ::cave::literals;
using namespace ::cave;
using cave::math::Vector2i;
using core::Color;
using core::Square;

static const char* ToString(SessionState p_state);

ChessGameSession::ChessGameSession(cave::IHostServices& p_host) noexcept
    : m_host(p_host)
    , m_state(SessionState::AwaitPlayerInput) {}

ChessGameSession::~ChessGameSession() = default;

void ChessGameSession::Tick() {
    switch (m_state) {
#define SESSION_STATE(Enum)  \
    case SessionState::Enum: \
        Tick##Enum();        \
        break;
        SESSION_STATE_LIST
#undef SESSION_STATE
    }

    if (m_auth->GameOver()) {
        SetState(SessionState::GameOver);
        return;
    }

    m_host.intentDispatcher().Flush();

    // update client visual
    m_client->Present();

    // @TODO: refactor this part
    if (m_selector) {
        Vector2i focused = m_selector->GetFocused();
        Square focused_sq = Square::FromFileRank((uint8_t)focused.x, (uint8_t)focused.y);
        m_client->Presenter().SetFocusedSquare(focused_sq);
    }
}

void ChessGameSession::TickAwaitPlayerInput() {
    // @TODO: grid adapter should be owned by client/player?
    if (m_grid_adapter) {
        m_grid_adapter->tick(m_host.gameInput());
    }

    // poll player intents
    const PlayerId player = m_auth->CurrentPlayer();
    m_agents[std::to_underlying(player)]->tick(m_host);
}

void ChessGameSession::TickResolvingMove() {
    SetState(SessionState::Animating);
}

void ChessGameSession::TickAnimating() {
    if (IsAnimating()) {
        return;
    }

    SetState(SessionState::AwaitPlayerInput);
}

void ChessGameSession::TickGameOver() {
    if (IsAnimating()) {
        return;
    }

    m_host.log().Info(LogChannel::Game, "Game Over!");

    auto state = std::make_unique<GameOverState>();
    m_host.intentDispatcher().Queue<ChessStateIntent>(std::move(state));
}

bool ChessGameSession::IsAnimating() const {
    auto& query = m_host.sceneQuery();
    return query.GetComponentCount(TransformAnimationComponent_Id) != 0;
}

std::unique_ptr<IPlayerAgent> ChessGameSession::CreatePlayer(PlayerId p_id,
                                                             PlayerKind p_kind) {
    switch (p_kind) {
        case PlayerKind::LocalHuman:
            return std::make_unique<LocalHumanAgent>(p_id);
        case PlayerKind::LocalAI:
            return std::make_unique<ChessAIAgent>(p_id, *m_client);
        case PlayerKind::RemoteNetwork:
            return nullptr;
        default:
            return nullptr;
    }
}

// @TODO: this should be configured by MainMenu?
void ChessGameSession::OnEnterBoot() {
    MatchConfig config{};
    config.black = { PlayerKind::LocalAI };

    m_auth = std::make_unique<ChessMatchAuthority>(m_host);
    m_client = std::make_unique<ChessGameClient>(m_host, *this, *m_auth);

    const PlayerKind white = config.white.kind;
    const PlayerKind black = config.black.kind;

    m_agents[0] = CreatePlayer(Color::White, white);
    m_agents[1] = CreatePlayer(Color::Black, black);

    const bool any_human = white == PlayerKind::LocalHuman || black == PlayerKind::LocalHuman;
    if (any_human) {
        m_grid_adapter = std::make_unique<ChessGridSelectorAdapter>(
            m_host.intentDispatcher(),
            m_host.log(),
            *m_client,
            m_client->Presenter());

        cave::GridSelectController::Callbacks cbs = {
            .can_select = [this](int x, int y) { return m_grid_adapter->canSelect(x, y); },
            .on_select = [this](int x, int y) { m_grid_adapter->onSelect(x, y); },
            .can_drop = [this](int sx, int sy, int dx, int dy) { return m_grid_adapter->canDrop(sx, sy, dx, dy); },
            .on_drop = [this](int sx, int sy, int dx, int dy) { m_grid_adapter->onDrop(sx, sy, dx, dy); },
            .on_cancel = [this]() { m_grid_adapter->onCancel(); },
            .on_invalid = [this](int sx, int sy, int dx, int dy) { m_grid_adapter->onInvalid(sx, sy, dx, dy); }
        };

        m_selector = std::make_unique<cave::GridSelectController>(
            Vector2i(8, 8),
            std::move(cbs));

        m_grid_adapter->setController(m_selector.get());

        m_grid_adapter->setPlayerCb([this](PlayerId id) -> LocalHumanAgent* {
            return dynamic_cast<LocalHumanAgent*>(m_agents[std::to_underlying(id)].get());
        });
    }

    m_client->OnBoot();
}

#if USING(DEBUG_BUILD)
static const char* ToString(SessionState p_state) {
    switch (p_state) {
#define SESSION_STATE(Enum)  \
    case SessionState::Enum: \
        return #Enum;
        SESSION_STATE_LIST
#undef SESSION_STATE
        default:
            return "?";
    }
}
#endif

void ChessGameSession::SetState(SessionState p_state) {
    if (p_state == m_state) {
        return;
    }

#if USING(DEBUG_BUILD)
    auto msg = std::format("ChessState {} -> {}", ToString(m_state), ToString(p_state));
    m_host.log().Trace(LogChannel::Game, std::move(msg));
#endif

    m_state = p_state;
}

}  // namespace chess
