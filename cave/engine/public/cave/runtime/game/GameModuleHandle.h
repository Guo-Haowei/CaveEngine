// =============================================================================
// File: cave/runtime/game/GameModuleHandle.h
// =============================================================================
#pragma once
#include "cave/platform/Dll.h"
#include "cave/runtime/game/IGameModule.h"

namespace cave {

class NativeScriptRegistry;

class GameModuleHandle {
public:
    using CreateFn = IGameModule* (*)();
    using DestroyFn = void (*)(IGameModule*);

    bool loadFromDll(const char* dll_path, NativeScriptRegistry& registry);

    void unload();
    bool loaded() const { return m_module != nullptr; }

    IGameModule* get() const { return m_module; }

private:
    Dll m_dll{};

    CreateFn m_create_func = nullptr;
    DestroyFn m_destroy_func = nullptr;

    IGameModule* m_module = nullptr;
};

}  // namespace cave