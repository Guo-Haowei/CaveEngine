#pragma once
#include "ChessGame.h"
#include "ChessGrideSelectorAdapter.h"
#include "ChessPresenter.h"

#include "cave/runtime/controller/GridSelectController.h"

// clang-format off
namespace cave { class IHostServices; }
namespace cave { class IInputService; }
// clang-format on

namespace chess {

class ChessMode {
public:
    ChessMode();

    void OnGameBegin(cave::IHostServices& p_host);

    void OnGameEnd(cave::IHostServices& p_host);

    void Tick(cave::IHostServices& p_host);

private:
    void ProcessInput(cave::IInputService& p_input);

    ChessGame m_game;
    ChessGridSelectorAdapter m_grid_adapter;
    ChessPresenter m_presenter;
    std::unique_ptr<cave::GridSelectController> m_selector;
};

}  // namespace chess
