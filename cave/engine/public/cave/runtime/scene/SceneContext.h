// =============================================================================
// File: cave/runtime/scene/SceneContext.h
// =============================================================================
#pragma once
#include "cave/runtime/scene/SceneQuery.h"

namespace cave {

struct EngineServices;
class Scene;
class NativeScriptRegistry;

struct SceneContext {
    NativeScriptRegistry& native_scripts;
    Scene& scene;
    SceneQuery query;

    // @TODO: do not expose all services
    EngineServices& engine_services;
};

}  // namespace cave
