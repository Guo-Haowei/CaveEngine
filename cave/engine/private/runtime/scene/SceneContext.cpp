#include "cave/runtime/scene/SceneContext.h"
#include "cave/runtime/scene/SceneRuntime.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

SceneContext::SceneContext(Scene& scene, RuntimeServices& services)
    : scene{ scene }
    , runtime{ *scene.runtime() }
    , query{ scene }
    , services{ services } {
}

void* SceneContext::system(SceneSystemId system_id) {
    return runtime.systems().get(system_id);
}

}  // namespace cave
