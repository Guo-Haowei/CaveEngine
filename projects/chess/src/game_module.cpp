#include <memory>

#include <cave/plugin/game_module_api.h>
#include <cave/runtime/framework/IApplication.h>
#include <cave/runtime/gameplay/GameModeFactory.h>

#include "ChessMode.h"

namespace cave::chess {

static void RegisterGame(IApplication& p_app, const GameLoadArgs& p_args) {
    (void)p_args;

    p_app.GetGameModeFactory().Register("chess", []() -> std::unique_ptr<IGameMode> {
        return std::make_unique<ChessMode>();
    });
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
