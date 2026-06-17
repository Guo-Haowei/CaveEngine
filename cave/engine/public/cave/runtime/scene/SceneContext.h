// =============================================================================
// File: cave/runtime/scene/SceneContext.h
// =============================================================================
#pragma once

namespace cave {

struct EngineServices;
class Scene;
class NativeScriptRegistry;

// @TODO: do not expose all services
struct SceneContext {
    EngineServices& engine_services;
    NativeScriptRegistry& native_scripts;
    Scene& scene;
};

}  // namespace cave
