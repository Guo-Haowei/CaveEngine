#include "MainMenuState.h"

#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/framework/IUIRuntime.h"
#include "cave/runtime/intent/IntentBus.h"
#include "cave/runtime/scene/SceneRuntime.h"

#include "chess/game/ChessIntent.h"
#include "chess/states/GameplayState.h"

namespace chess {

using namespace cave;

void MainMenuState::onEnter() {
}

void MainMenuState::tick(float) {
    IUIRuntime& ui = m_runtime.services().UI();

    ui.beginView(m_runtime.viewId());
    const float offset_x = 760.0f;
    const float offset_y = 200.0f;
    if (ui.button(1, { offset_x, offset_y, 400, 100 })) {
        auto gameplay = std::make_unique<GameplayState>(m_runtime, m_intent_bus);
        m_intent_bus.queue<ChessStateIntent>(std::move(gameplay));
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
