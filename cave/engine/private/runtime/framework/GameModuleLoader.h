// =============================================================================
// File: engine/private/runtime/framework/GameModuleLoader.h
// =============================================================================
#pragma once
#include "cave/plugin/game_module_api.h"

namespace cave {

struct LoadedGameModule {
    void* handle = nullptr;
    const GameModuleApi* api = nullptr;
};

bool LoadGameModule(const char* p_dll_path, LoadedGameModule& p_out_module);
void UnloadGameModule(LoadedGameModule& p_module);

}  // namespace cave