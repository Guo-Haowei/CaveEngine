// =============================================================================
// File: cave/game/IGameModule.h
// =============================================================================
#pragma once
#include <cstdint>
#include "cave/runtime/script/native/NativeScriptRegistry.h"

namespace cave {

enum class AppMode : uint8_t {
    Client,
    Server,
    Editor,
};

class IGameModule {
public:
    virtual ~IGameModule() = default;

    virtual void registerNativeScripts(NativeScriptRegistry&) {}
};

}  // namespace cave
