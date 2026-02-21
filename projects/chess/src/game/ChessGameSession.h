#pragma once
#include "ChessGameClient.h"
#include "ChessMatchAuthority.h"

namespace chess {

class ChessGameSession {
public:
    ChessGameSession();

    void OnGameBegin(cave::IHostServices& p_host);

    void OnGameEnd(cave::IHostServices& p_host);

    void Tick(cave::IHostServices& p_host);

private:
    ChessMatchAuthority m_auth;
    ChessGameClient m_client;
};

}  // namespace chess
