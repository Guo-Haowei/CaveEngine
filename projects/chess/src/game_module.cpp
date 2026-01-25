#include <cave/plugin/game_module_api.h>
#include <cave/runtime/framework/IApplication.h>
#include <cave/runtime/gameplay/GameModeFactory.h>

#include "ChessMode.h"

namespace cave::chess {

static void RegisterGame(IApplication& p_app, const GameLoadArgs& p_args) {
    (void)p_args;

    p_app.GetGameModeFactory().Register(
        "chess",
        []() -> IGameMode* { return new ChessGameMode(); },
        [](IGameMode* p_mode) { delete p_mode; });
}

static const GameModuleApi g_api = {
    CAVE_GAME_MODULE_API_VERSION,
    "ChessGame",
    RegisterGame,
};

}  // namespace cave::chess

extern "C" CAVE_API const cave::GameModuleApi* Cave_GetGameModuleApi() {
    return &cave::chess::g_api;
}
