// =============================================================================
// File: cave/runtime/game/IGameModule.h
// =============================================================================
#pragma once
#include <cstdint>
#include "cave/runtime/script/native/NativeScriptRegistry.h"

namespace cave {

class GameSession;

enum class AppMode : uint8_t {
    Client,
    Server,
    Editor,
};

class IGameModule {
public:
    virtual ~IGameModule() = default;

    virtual void registerNativeScripts(NativeScriptRegistry&) {}

    virtual bool startSession(GameSession&) { return true; }
    virtual void endSession(GameSession&) {}
};

}  // namespace cave
