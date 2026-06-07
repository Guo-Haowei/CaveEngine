#include "PIEHostServices.h"

#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/scene/SceneCommandPlayback.h"
#include "engine/private/core/diagnostics/log_sink/CompositeLogger.h"
#include "engine/private/runtime/framework/Engine.h"
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"

namespace cave {

PIEHostServices::PIEHostServices(IApplication& app,
                                 Scene& scene,
                                 ViewId view_id) noexcept
    : app_(app)
    , logger_(CompositeLogger::GetSingleton())
    , scene_(scene)
    , view_id_(view_id)
    , query_(scene)
    , writer_(*app.GetAssetRegistry()) {
}

AssetRegistry& PIEHostServices::assetRegistry() {
    return *app_.GetAssetRegistry();
}

ecs::ComponentRegistry& PIEHostServices::componentRegistry() {
    return engine::GetComponentRegistry();
}

IntentDispatcher& PIEHostServices::intentDispatcher() {
    return *app_.IntentDispatcher();
}

const IGameInput& PIEHostServices::gameInput() const {
    return app_.InputService().gameInput();
}

IUIRuntime& PIEHostServices::ui() {
    return *app_.UIService();
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
