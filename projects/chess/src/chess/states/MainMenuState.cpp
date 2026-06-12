#include "MainMenuState.h"

#include "cave/game/IHostServices.h"
#include "cave/runtime/framework/IUIRuntime.h"
#include "cave/runtime/intent/IntentDispatcher.h"

#include "chess/game/ChessIntent.h"
#include "chess/states/GameplayState.h"

namespace chess {

using namespace cave;

void MainMenuState::OnEnter(cave::IHostServices&) {
}

void MainMenuState::Tick(cave::IHostServices& host, const cave::FrameTime&) {
    IUIRuntime& ui = host.ui();

    ui.beginView(host.viewId());
    const float offset_x = 760.0f;
    const float offset_y = 200.0f;
    if (ui.button(1, { offset_x, offset_y, 400, 100 })) {
        auto gameplay = std::make_unique<GameplayState>();
        host.intentDispatcher().queue<ChessStateIntent>(std::move(gameplay));
    }
    if (ui.button(2, { offset_x, offset_y + 200, 400, 100 })) {
        LOG_OK(LogChannel::Game, "UI Button 2 clicked");
    }
    if (ui.button(3, { offset_x, offset_y + 400, 400, 100 })) {
        LOG_OK(LogChannel::Game, "UI Button 3 clicked");
    }
    ui.endView();
}

}  // namespace chess
