#pragma once
#include "cave/runtime/controller/GridSelectController.h"

#include "ChessGameClient.h"
#include "ChessGrideSelectorAdapter.h"
#include "ChessMatchAuthority.h"

namespace chess {

class IPlayerAgent;

class ChessGameSession {
public:
    ChessGameSession();
    ~ChessGameSession();

    void OnGameBegin(cave::IHostServices& p_host);

    void OnGameEnd(cave::IHostServices& p_host);

    void Tick(cave::IHostServices& p_host);

private:
    ChessMatchAuthority m_auth;
    ChessGameClient m_client;

    std::unique_ptr<cave::GridSelectController> m_selector;
    ChessGridSelectorAdapter m_grid_adapter;

    std::unique_ptr<IPlayerAgent> m_white_player;
    std::unique_ptr<IPlayerAgent> m_black_player;
};

}  // namespace chess
