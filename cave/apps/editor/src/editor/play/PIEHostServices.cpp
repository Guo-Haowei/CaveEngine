#include "PIEHostServices.h"

#include "cave/runtime/framework/IApplication.h"
#include "engine/private/core/diagnostics/logger/Logger.h"
#include "engine/private/runtime/framework/Engine.h"

namespace cave {

PIEHostServices::PIEHostServices(IApplication& p_app, Scene& p_scene) noexcept
    : m_app(p_app)
    , m_query(p_scene)
    , m_writer(*p_app.GetAssetRegistry()) {
}

ILogger& PIEHostServices::Log() {
    return CompositeLogger::GetSingleton();
}

AssetRegistry& PIEHostServices::AssetRegistry() {
    return *m_app.GetAssetRegistry();
}

ecs::ComponentRegistry& PIEHostServices::ComponentRegistry() {
    return engine::GetComponentRegistry();
}

}  // namespace cave
