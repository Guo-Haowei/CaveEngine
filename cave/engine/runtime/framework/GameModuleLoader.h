// =============================================================================
// File: engine/runtime/framework/GameModuleLoader.h
// =============================================================================
#pragma once
#include "sdk/cave/api/GameModule.h"

namespace cave {

struct LoadedGameModule {
    void* handle = nullptr;
    const GameModuleApi* api = nullptr;
};

bool TryLoadGameModule(const char* p_dll_path, LoadedGameModule& p_out_module);
void UnloadGameModule(LoadedGameModule& p_module);

}  // namespace cave