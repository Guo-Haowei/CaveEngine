#pragma once
#include <stdint.h>

namespace cave {

class Application;

enum class HostKind : uint32_t {
    Editor = 0,
    Game = 1,
    Server = 2,
};

struct GameLoadArgs {
    HostKind hostKind;
    const char* projectPath;
};

static constexpr uint32_t CAVE_GAME_MODULE_API_VERSION = 1;

struct GameModuleApi {
    uint32_t apiVersion;
    const char* moduleName;

    void (*RegisterGame)(Application& app, const GameLoadArgs& args);  // optional
};

}  // namespace cave

extern "C" {

__declspec(dllexport) const cave::GameModuleApi* Cave_GetGameModuleApi();

}