#pragma once
#include <stdint.h>

#include "cave/Export.h"

namespace cave {

class Application;

enum class HostKind : uint32_t {
    Editor = 0,
    Game = 1,
    Server = 2,
};

struct GameLoadArgs {
    HostKind host_kind;
    const char* project_path;
};

constexpr uint32_t CAVE_GAME_MODULE_API_VERSION = 1;

struct GameModuleApi {
    uint32_t version;
    const char* module_name;
    void (*RegisterGame)(Application& p_app, const GameLoadArgs& p_args);
};

}  // namespace cave

extern "C" CAVE_API const cave::GameModuleApi* Cave_GetGameModuleApi();
