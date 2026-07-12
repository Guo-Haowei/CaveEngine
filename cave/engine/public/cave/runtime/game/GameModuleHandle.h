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
    bool loaded() const { return module_ != nullptr; }

    IGameModule* get() const { return module_; }

private:
    struct ModuleDeleter {
        void operator()(IGameModule* p) const { delete p; }
    };

    Dll dll_{};

    CreateFn create_ = nullptr;
    DestroyFn destroy_ = nullptr;

    IGameModule* module_ = nullptr;
};

}  // namespace cave