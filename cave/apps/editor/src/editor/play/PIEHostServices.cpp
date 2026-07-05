#include "PIEHostServices.h"

#include "cave/core/diagnostics/CompositeLogger.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/scene/SceneCommandPlayback.h"

#include "engine/private/runtime/framework/Engine.h"
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"

namespace cave {

PIEHostServices::PIEHostServices(EngineServices& services,
                                 Scene& scene,
                                 ViewId view_id) noexcept
    : services_(services)
    , scene_(scene)
    , view_id_(view_id)
    , writer_(services.assetRegistry())
    , scene_query_(scene)
    , view_query_(services.viewManager()) {
}

AssetRegistry& PIEHostServices::assetRegistry() {
    return services_.assetRegistry();
}

ecs::ComponentRegistry& PIEHostServices::componentRegistry() {
    return engine::GetComponentRegistry();
}

IDebugDrawService& PIEHostServices::debugDraw() {
    return services_.debugDraw();
}

DisplayService& PIEHostServices::displayService() {
    return services_.displayService();
}

IntentDispatcher& PIEHostServices::intentDispatcher() {
    return services_.intentDispatcher();
}

const IGameInput& PIEHostServices::gameInput() const {
    return services_.inputService().gameInput();
}

IUIRuntime& PIEHostServices::ui() {
    return services_.ui();
}

void PIEHostServices::flushSceneCommands() {
    SceneCommandBuffer& cb = sceneWriter();
    if (cb.bytes()) {
        SceneCommandExecutor executor(scene_);
        EntityMap map(cb.allocationCount());
        SceneCommandPlayback::Play(cb, executor, { map, scene_ });
        cb.reset();
    }
}

}  // namespace cave
