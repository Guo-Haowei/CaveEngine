// =============================================================================
// File: engine/public/cave/game/GameModuleHandle.h
// =============================================================================
#pragma once
#include <memory>

#include "cave/platform/Dll.h"
#include "cave/game/IGameModule.h"

namespace cave {

class GameModuleHandle {
public:
    using CreateFn = IGameModule* (*)();

    bool LoadFromDll(const char* dll_path);

    void Unload();

    IGameModule* Get() const { return m_module.get(); }

private:
    struct ModuleDeleter {
        void operator()(IGameModule* p) const { delete p; }
    };

    Dll m_dll{};
    CreateFn m_create = nullptr;
    std::unique_ptr<IGameModule, ModuleDeleter> m_module{};
};

}  // namespace cave