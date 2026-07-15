#pragma once
#include "cave/runtime/game/IGameModule.h"

namespace super_cave_boy {

class SuperCaveBoy final : public cave::IGameModule {
public:
    SuperCaveBoy();
    ~SuperCaveBoy();

    void registerNativeScripts(cave::NativeScriptRegistry& registry) override;

    bool startSession(cave::GameSession& session) override;
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
