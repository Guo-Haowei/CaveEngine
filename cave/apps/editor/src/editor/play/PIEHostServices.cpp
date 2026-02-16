#include "PIEHostServices.h"

#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/scene/SceneCommandPlayback.h"
#include "engine/private/core/diagnostics/logger/Logger.h"
#include "engine/private/runtime/framework/Engine.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"

namespace cave {

PIEHostServices::PIEHostServices(IApplication& p_app, Scene& p_scene) noexcept
    : m_app(p_app)
    , m_scene(p_scene)
    , m_query(p_scene)
    , m_writer(*p_app.GetAssetRegistry()) {
}

AssetRegistry& PIEHostServices::AssetRegistry() {
    return *m_app.GetAssetRegistry();
}

ecs::ComponentRegistry& PIEHostServices::ComponentRegistry() {
    return engine::GetComponentRegistry();
}

IInputService& PIEHostServices::Input() {
    return *m_app.InputService();
}

ILogger& PIEHostServices::Log() {
    return CompositeLogger::GetSingleton();
}

void PIEHostServices::FlushSceneCommands() {
    SceneCommandBuffer& cb = SceneWriter();
    if (cb.Data()) {
        SceneCommandExecutor executor(m_scene);
        EntityMap map(cb.GetAllocationCount());
        SceneCommandPlayback(cb, executor, map);
    }
}

}  // namespace cave
