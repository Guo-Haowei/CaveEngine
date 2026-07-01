#pragma once
#include <memory>
#include "cave/core/typedefs.h"
#include "cave/game/IGameModule.h"

namespace super_cave_boy {

class SuperCaveBoy final : public cave::IGameModule {
public:
    SuperCaveBoy();
    ~SuperCaveBoy();

    void registerNativeScripts(cave::NativeScriptRegistry& registry) override;

    void onModuleLoaded(cave::IHostServices& host) override;
    void onModuleUnloaded(cave::IHostServices& host) override;

    void onGameBegin(cave::IHostServices& host) override;
    void onGameEnd(cave::IHostServices& host) override;

    void tick(cave::IHostServices& host, const cave::FrameTime& time) override;
};

}  // namespace super_cave_boy

extern "C" {

CAVE_API cave::IGameModule* CreateGameModule() {
    return new ::super_cave_boy::SuperCaveBoy();
}

CAVE_API void DestroyGameModule(::cave::IGameModule* game) {
    if (game != nullptr) {
        delete game;
    }
}
}
