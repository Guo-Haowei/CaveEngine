#pragma once
#include <memory>
#include "cave/core/typedefs.h"
#include "cave/game/IGameModule.h"

namespace super_cave_boy {

class PlayerController;

class GameModule final : public cave::IGameModule {
public:
    GameModule();
    ~GameModule();

    void registerNativeScripts(cave::NativeScriptRegistry& registry) override;

    void onModuleLoaded(cave::IHostServices& host) override;
    void onModuleUnloaded(cave::IHostServices& host) override;

    void onGameBegin(cave::IHostServices& host) override;
    void onGameEnd(cave::IHostServices& host) override;

    void tick(cave::IHostServices& host, const cave::FrameTime& time) override;

private:
    std::unique_ptr<PlayerController> controller_;
};

}  // namespace super_cave_boy

extern "C" {

CAVE_API cave::IGameModule* CreateGameModule() {
    return new ::super_cave_boy::GameModule();
}

CAVE_API void DestroyGameModule(::cave::IGameModule* game) {
    if (game != nullptr) {
        delete game;
    }
}
}
