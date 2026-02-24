#include "ChessGameSession.h"

#include "cave/game/IHostServices.h"
#include "cave/runtime/controller/GridSelectController.h"

#include "ChessAIAgent.h"
#include "ChessGameClient.h"
#include "ChessGridSelectorAdapter.h"
#include "ChessMatchAuthority.h"
#include "LocalHumanAgent.h"

namespace chess {

using cave::math::Vector2i;
using core::Square;

ChessGameSession::ChessGameSession() noexcept = default;
ChessGameSession::~ChessGameSession() = default;

#if 0

#endif

void ChessGameSession::Tick(cave::IHostServices& p_host) {
    switch (m_state) {
        case SessionState::Boot:
            TickBoot(p_host);
            break;
        case SessionState::Playing:
            TickPlaying(p_host);
            break;
        case SessionState::GameOver:
            TickGameOver(p_host);
            break;
    }
}

std::unique_ptr<IPlayerAgent> ChessGameSession::CreatePlayer(PlayerId p_id,
                                           PlayerKind p_kind) {
    switch (p_kind) {
        case PlayerKind::LocalHuman:
            return std::make_unique<LocalHumanAgent>(p_id, *m_auth);
        case PlayerKind::LocalAI:
            return std::make_unique<ChessAIAgent>(p_id, *m_auth, *m_client);
        case PlayerKind::RemoteNetwork:
            return nullptr;
        default:
            return nullptr;
    }
}

void ChessGameSession::EnterBoot(cave::IHostServices& p_host) {
    MatchConfig config{};
    config.black = { PlayerKind::LocalAI };

    m_auth = std::make_unique<ChessMatchAuthority>();
    m_client = std::make_unique<ChessGameClient>(*m_auth);

    const PlayerKind white = config.white.kind;
    const PlayerKind black = config.black.kind;

    m_white_player = CreatePlayer(0, white);
    m_black_player = CreatePlayer(1, black);

    const bool any_human = white == PlayerKind::LocalHuman || black == PlayerKind::LocalHuman;
    if (any_human) {
        m_grid_adapter = std::make_unique<ChessGridSelectorAdapter>(*m_client, m_client->Presenter());

        cave::GridSelectController::Callbacks cbs = {
            .can_select = [this](int x, int y) { return m_grid_adapter->CanSelect(x, y); },
            .on_select = [this](int x, int y) { m_grid_adapter->OnSelect(x, y); },
            .can_drop = [this](int sx, int sy, int dx, int dy) { return m_grid_adapter->CanDrop(sx, sy, dx, dy); },
            .on_drop = [this](int sx, int sy, int dx, int dy) { m_grid_adapter->OnDrop(sx, sy, dx, dy); },
            .on_cancel = [this]() { m_grid_adapter->OnCancel(); },
            .on_invalid = [this](int sx, int sy, int dx, int dy) { m_grid_adapter->OnInvalid(sx, sy, dx, dy); }
        };

        m_selector = std::make_unique<cave::GridSelectController>(
            Vector2i(8, 8),
            std::move(cbs));

        m_grid_adapter->SetController(m_selector.get());

        m_grid_adapter->SetGetPlayerFunc([this](PlayerId p_id) -> LocalHumanAgent* {
            return dynamic_cast<LocalHumanAgent*>(p_id == 0 ? m_white_player.get() : m_black_player.get());
        });
    }

    m_client->OnBoot(p_host);
}

void ChessGameSession::TickBoot(cave::IHostServices& p_host) {
    EnterBoot(p_host);

    m_state = SessionState::Playing;
}

void ChessGameSession::TickPlaying(cave::IHostServices& p_host) {
    if (m_grid_adapter) {
        m_grid_adapter->Tick(p_host.Input());
    }

    m_white_player->Tick();
    m_black_player->Tick();

    if (m_selector) {
        Vector2i focused = m_selector->GetFocused();
        Square focused_sq = Square::FromFileRank((uint8_t)focused.x, (uint8_t)focused.y);
        m_client->Presenter().SetFocusedSquare(focused_sq);
    }

    m_auth->Tick();
    if (m_auth->GameOver()) {
        m_state = SessionState::GameOver;
        return;
    }

    m_client->Tick(p_host);
}

void ChessGameSession::TickGameOver(cave::IHostServices& p_host) {
    using namespace cave;

    p_host.Log().Print(LOG_LEVEL_OK, "GameOver!");
}

}  // namespace chess
