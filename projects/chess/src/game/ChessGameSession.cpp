#include "ChessGameSession.h"

#include "cave/game/IHostServices.h"
#include "LocalHumanAgent.h"

namespace chess {

using cave::math::Vector2i;
using core::Square;

ChessGameSession::ChessGameSession()
    : m_auth{}
    , m_client{ m_auth }
    , m_grid_adapter{ m_client, m_client.Presenter() } {

    cave::GridSelectController::Callbacks cbs = {
        .can_select = [this](int x, int y) { return m_grid_adapter.CanSelect(x, y); },
        .on_select = [this](int x, int y) { m_grid_adapter.OnSelect(x, y); },
        .can_drop = [this](int sx, int sy, int dx, int dy) { return m_grid_adapter.CanDrop(sx, sy, dx, dy); },
        .on_drop = [this](int sx, int sy, int dx, int dy) { m_grid_adapter.OnDrop(sx, sy, dx, dy); },
        .on_cancel = [this]() { m_grid_adapter.OnCancel(); },
        .on_invalid = [this](int sx, int sy, int dx, int dy) { m_grid_adapter.OnInvalid(sx, sy, dx, dy); }
    };

    m_selector = std::make_unique<cave::GridSelectController>(
        Vector2i(8, 8),
        std::move(cbs));

    m_grid_adapter.SetController(m_selector.get());

    // @TODO: this is bad, fix
    // @NOTE: maybe use lambda instead
    LocalHumanAgent* white = new LocalHumanAgent(0, m_auth);
    LocalHumanAgent* black = new LocalHumanAgent(1, m_auth);
    m_grid_adapter.SetPlayer(0, white);
    m_grid_adapter.SetPlayer(1, black);

    m_white_player.reset(white);
    m_black_player.reset(black);
}

ChessGameSession::~ChessGameSession() = default;

void ChessGameSession::OnGameBegin(cave::IHostServices& p_host) {
    m_client.OnGameBegin(p_host);
}

void ChessGameSession::OnGameEnd(cave::IHostServices& p_host) {
    m_client.OnGameEnd(p_host);
}

void ChessGameSession::Tick(cave::IHostServices& p_host) {
    m_grid_adapter.Tick(p_host.Input());
    m_white_player->Tick();
    m_black_player->Tick();

    Vector2i focused = m_selector->GetFocused();
    m_client.Presenter().SetSelectedSquare(
        Square::FromFileRank((uint8_t)focused.x, (uint8_t)focused.y));

    m_client.Tick(p_host);
    m_auth.Tick();
    // @TODO: post tick
}

}  // namespace chess
