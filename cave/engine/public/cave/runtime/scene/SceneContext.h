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
    Scene& scene;
    SceneRuntime& runtime;
    SceneQuery query;
    RuntimeServices& services;

    ViewId view_id;
    ISceneTransitionRequests* scene_transition{};

    SceneContext(Scene& scene, RuntimeServices& services);

    template<typename T>
    T* system() { return reinterpret_cast<T*>(system(T::kSystemId)); }

private:
    void* system(SceneSystemId system_id);
};

}  // namespace cave
