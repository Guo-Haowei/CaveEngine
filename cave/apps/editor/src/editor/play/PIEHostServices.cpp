#include "PIEHostServices.h"

#include "cave/core/diagnostics/CompositeLogger.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/scene/SceneCommandPlayback.h"

#include "engine/private/runtime/framework/Engine.h"
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"

namespace cave {

PIEHostServices::PIEHostServices(IApplication& app,
                                 Scene& scene,
                                 ViewId view_id) noexcept
    : app_(app)
    , scene_(scene)
    , view_id_(view_id)
    , writer_(app.services().assetRegistry())
    , scene_query_(scene)
    , view_query_(app.services().viewManager()) {
}

AssetRegistry& PIEHostServices::assetRegistry() {
    return app_.services().assetRegistry();
}

ecs::ComponentRegistry& PIEHostServices::componentRegistry() {
    return engine::GetComponentRegistry();
}

DisplayService& PIEHostServices::displayService() {
    return app_.services().displayService();
}

IntentDispatcher& PIEHostServices::intentDispatcher() {
    return app_.services().intentDispatcher();
}

const IGameInput& PIEHostServices::gameInput() const {
    return app_.services().inputService().gameInput();
}

IUIRuntime& PIEHostServices::ui() {
    return app_.services().ui();
}

void PIEHostServices::flushSceneCommands() {
    SceneCommandBuffer& cb = sceneWriter();
    if (cb.Data()) {
        SceneCommandExecutor executor(scene_);
        EntityMap map(cb.GetAllocationCount());
        SceneCommandPlayback::Play(cb, executor, { map, scene_ });
        cb.Reset();
    }
}

}  // namespace cave
