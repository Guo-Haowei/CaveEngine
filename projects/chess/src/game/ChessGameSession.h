#pragma once
#include "ChessGameClient.h"
#include "ChessGrideSelectorAdapter.h"
#include "ChessPresenter.h"

#include "cave/runtime/controller/GridSelectController.h"

// clang-format off
namespace cave { class IHostServices; }
namespace cave { class IInputService; }
// clang-format on

namespace chess {

class ChessGameSession {
public:
    ChessGameSession();

    void OnGameBegin(cave::IHostServices& p_host);

    void OnGameEnd(cave::IHostServices& p_host);

    void Tick(cave::IHostServices& p_host);

private:
    void ProcessInput(cave::IInputService& p_input);

    ChessGameClient m_client;
    ChessPresenter m_presenter;
    ChessGridSelectorAdapter m_grid_adapter;
    std::unique_ptr<cave::GridSelectController> m_selector;
};

}  // namespace chess
