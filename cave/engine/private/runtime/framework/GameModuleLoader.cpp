// =============================================================================
// File: engine/private/runtime/framework/GameModuleLoader.cpp
// =============================================================================
#include "GameModuleLoader.h"

#include "engine/private/core/string/StringUtils.h"

// @TODO: hide platform specific code from loader
#if USING(PLATFORM_WINDOWS)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace cave {

static void* LoadSymbol(void* p_handle, const char* p_name) {
#if USING(PLATFORM_WINDOWS)
    return ::GetProcAddress(reinterpret_cast<HMODULE>(p_handle), p_name);
#else
    return ::dlsym(p_handle, name);
#endif
}

static void LogLoadError(const char* p_dll_path) {
#if USING(PLATFORM_WINDOWS)
    const DWORD err = ::GetLastError();
    LOG_WARN("game module not loaded: '{}' (GetLastError={})", p_dll_path, (uint32_t)err);
#else
    const char* err = ::dlerror();
    err = err ? err : "unknown";
    LOG_WARN("game module not loaded: '{}' ({})", p_dll_path, err);
#endif
}

bool LoadGameModule(const char* p_dll_path, LoadedGameModule& p_out_module) {
    p_out_module = {};

    if (StringUtils::IsNullOrEmpty(p_dll_path)) {
        return false;
    }

#if USING(PLATFORM_WINDOWS)
    HMODULE mod = ::LoadLibraryA(p_dll_path);
    if (!mod) {
        // Optional dll: not an error for editor boot.
        LogLoadError(p_dll_path);
        return false;
    }

    p_out_module.handle = reinterpret_cast<void*>(mod);
#else
    // Clear old dlerror first
    (void)::dlerror();
    void* mod = ::dlopen(dllPath, RTLD_NOW);
    if (!mod) {
        LogLoadError(dllPath);
        return false;
    }
    p_out_module.handle = mod;
#endif

    using GetApiFn = const GameModuleApi* (*)();
    auto* get_api_symbol_fn = reinterpret_cast<GetApiFn>(LoadSymbol(p_out_module.handle, "Cave_GetGameModuleApi"));

    if (!get_api_symbol_fn) {
        LOG_ERROR("LoadGameModule: '{}' missing export: Cave_GetGameModuleApi", p_dll_path);
        UnloadGameModule(p_out_module);
        return false;
    }

    const GameModuleApi* api = get_api_symbol_fn();
    if (!api) {
        LOG_ERROR("LoadGameModule: '{}' returned null GameModuleApi", p_dll_path);
        UnloadGameModule(p_out_module);
        return false;
    }

    if (api->version != CAVE_GAME_MODULE_API_VERSION) {
        LOG_ERROR("LoadGameModule: '{}' api version mismatch (expected {}, got {})",
                  p_dll_path,
                  CAVE_GAME_MODULE_API_VERSION,
                  api->version);
        UnloadGameModule(p_out_module);
        return false;
    }

    p_out_module.api = api;

    LOG_OK("LoadGameModule: module loaded: {}", !StringUtils::IsNullOrEmpty(api->module_name) ? api->module_name : "<unnamed>");
    return true;
}

void UnloadGameModule(LoadedGameModule& p_module) {
    // Important: mod.api points into the DLL, invalidate before unloading.
    p_module.api = nullptr;

    if (p_module.handle) {
#if USING(PLATFORM_WINDOWS)
        ::FreeLibrary(reinterpret_cast<HMODULE>(p_module.handle));
#else
        ::dlclose(p_module.handle);
#endif
    }

    p_module.handle = nullptr;
}

}  // namespace cave
