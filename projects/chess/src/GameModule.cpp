#include "cave/api/GameModule.h"

namespace cave::chess {

static void RegisterGame(GameRuntimeState& p_app, const GameLoadArgs& p_args) {
    // Register chess app states
    // app.GetStateMachine().RegisterState<ChessPlayState>();

    // Register chess game modes
    // app.GetGameModeFactory().Register("chess", [] {
    //     return std::make_unique<ChessGameMode>();
    // });

    // Register Lua bindings
    // app.GetScriptManager().RegisterModule("chess", RegisterChessLua);

    (void)p_args;
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
