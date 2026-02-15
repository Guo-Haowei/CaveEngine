#include "PIEHostServices.h"

#include "engine/private/core/diagnostics/logger/Logger.h"
#include "cave/runtime/framework/IApplication.h"

namespace cave {

PIEHostServices::PIEHostServices(IApplication& p_app, SceneId p_pie_scene)
    : m_app(p_app)
    , m_pie_scene(p_pie_scene) {
}

ILogger& PIEHostServices::Log() {
    return CompositeLogger::GetSingleton();
}

AssetRegistry& PIEHostServices::AssetRegistry() {
    return *m_app.GetAssetRegistry();
}

}  // namespace cave
