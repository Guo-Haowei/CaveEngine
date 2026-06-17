// =============================================================================
// File: cave/runtime/scene/SceneContext.h
// =============================================================================
#pragma once

namespace cave {

struct EngineServices;
class Scene;
class NativeScriptRegistry;
class IGameInput;

struct SceneContext {
    const IGameInput& game_input;
    NativeScriptRegistry& native_scripts;
    Scene& scene;

    // @TODO: do not expose all services
    EngineServices& engine_services;
};

}  // namespace cave
