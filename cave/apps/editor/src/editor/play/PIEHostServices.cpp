#include "PIEHostServices.h"

#include "engine/private/core/diagnostics/logger/Logger.h"

namespace cave {

PIEHostServices::PIEHostServices(IApplication& p_app, SceneId p_pie_scene)
    : m_app(p_app)
    , m_pie_scene(p_pie_scene) {
}

ILogger& PIEHostServices::Log() {
    return CompositeLogger::GetSingleton();
}

}  // namespace cave
