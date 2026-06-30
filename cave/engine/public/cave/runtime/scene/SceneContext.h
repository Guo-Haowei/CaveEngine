// =============================================================================
// File: cave/runtime/scene/SceneContext.h
// =============================================================================
#pragma once
#include "cave/runtime/scene/SceneQuery.h"

namespace cave {

struct EngineServices;
class Scene;
class NativeScriptRegistry;
class ISceneOwner;

struct SceneContext {
    NativeScriptRegistry& native_scripts;
    Scene& scene;
    ISceneOwner& scene_owner;
    SceneQuery query;

    // @TODO: do not expose all services
    EngineServices& engine_services;
};

}  // namespace cave
