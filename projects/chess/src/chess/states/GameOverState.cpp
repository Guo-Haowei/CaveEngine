#include "GameOverState.h"

#include "cave/runtime/framework/IUIRuntime.h"
#include "cave/runtime/intent/IntentDispatcher.h"

#include "chess/game/ChessIntent.h"
#include "chess/states/MainMenuState.h"

namespace chess {

using namespace cave;

#if 0
void GameOverState::Tick(cave::IHostServices& p_host, const cave::FrameTime&) {
    cave::IUIRuntime& ui = p_host.ui();

    ui.beginView(p_host.viewId());
    const float offset_x = 760.0f;
    const float offset_y = 200.0f;
    if (ui.button(4, { offset_x, offset_y, 400, 100 })) {
        auto gameplay = std::make_unique<MainMenuState>();
        p_host.intentDispatcher().queue<ChessStateIntent>(std::move(gameplay));
    }
    if (ui.button(5, { offset_x, offset_y + 200, 400, 100 })) {
        LOG_OK(LogChannel::Game, "UI Button 2 clicked");
    }
    ui.endView();
}
#endif

}  // namespace chess
