#include "ChessGameSession.h"

namespace chess {

using cave::math::Vector2i;

ChessGameSession::ChessGameSession()
    : m_auth{}
    , m_client{} {
}

void ChessGameSession::OnGameBegin(cave::IHostServices& p_host) {
    m_client.OnGameBegin(p_host);
}

void ChessGameSession::OnGameEnd(cave::IHostServices& p_host) {
    m_client.OnGameEnd(p_host);
}

void ChessGameSession::Tick(cave::IHostServices& p_host) {
    m_client.Tick(p_host);
}

}  // namespace chess
