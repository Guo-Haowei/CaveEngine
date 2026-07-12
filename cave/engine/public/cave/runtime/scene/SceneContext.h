// =============================================================================
// File: cave/runtime/scene/SceneContext.h
// =============================================================================
#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace cave {

enum class SceneSystemId : uint32_t;

struct RuntimeServices;
class Scene;
class SceneRuntime;
class NativeScriptRegistry;
class ISceneTransitionRequests;

struct SceneContext {
    ISceneTransitionRequests* scene_transition{};
};

}  // namespace cave
