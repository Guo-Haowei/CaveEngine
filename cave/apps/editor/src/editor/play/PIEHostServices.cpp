#include "PIEHostServices.h"

#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/scene/SceneCommandPlayback.h"
#include "engine/private/core/diagnostics/log_sink/CompositeLogger.h"
#include "engine/private/runtime/framework/Engine.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"

namespace cave {

PIEHostServices::PIEHostServices(IApplication& p_app,
                                 Scene& p_scene,
                                 ViewId p_view_id) noexcept
    : m_app(p_app)
    , m_scene(p_scene)
    , m_view_id(p_view_id)
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
    return m_app.InputService();
}

IUIRuntime& PIEHostServices::UI() {
    return *m_app.UIService();
}

ILogSink& PIEHostServices::Log() {
    return CompositeLogger::GetSingleton();
}

void PIEHostServices::FlushSceneCommands() {
    SceneCommandBuffer& cb = SceneWriter();
    if (cb.Data()) {
        SceneCommandExecutor executor(m_scene);
        EntityMap map(cb.GetAllocationCount());
        SceneCommandPlayback::Play(cb, executor, { map, m_scene });
        cb.Reset();
    }
}

}  // namespace cave
