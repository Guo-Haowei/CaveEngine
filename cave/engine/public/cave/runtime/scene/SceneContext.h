// =============================================================================
// File: cave/runtime/scene/SceneContext.h
// =============================================================================
#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace cave {

struct EngineServices;
class Scene;
class NativeScriptRegistry;
class ISceneTransitionRequests;

struct SceneContext {
    Scene& scene;
    SceneQuery query;
    EngineServices& engine_services;

    ViewId view_id;
    ISceneTransitionRequests* scene_transition{};
};

}  // namespace cave
