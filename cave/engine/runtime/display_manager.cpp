#include "display_manager.h"

#include "engine/drivers/glfw/glfw_display_manager.h"
#include "engine/runtime/application.h"
#include "engine/runtime/common_dvars.h"
#if USING(PLATFORM_WINDOWS)
#include "engine/drivers/windows/win32_display_manager.h"
#endif
#include "engine/renderer/graphics_dvars.h"

namespace cave {

Result<void> IDisplayManager::InitializeImpl() {
    InitializeKeyMapping();

    const auto& spec = m_app->GetSpecification();

    std::string title{ spec.name };
    switch (spec.backend) {
#define BACKEND_DECLARE(ENUM, STR, ...) \
    case Backend::ENUM:                 \
        title.append(" [" STR "|");     \
        break;
        BACKEND_LIST
#undef BACKEND_DECLARE
        default:
            break;
    }

    title.append(
#if USING(DEBUG_BUILD)
        "Debug]"
#else
        "Release]"
#endif
    );

    WindowSpecfication info = {
        .title = std::move(title),
        .width = spec.width,
        .height = spec.height,
        .backend = spec.backend,
        .decorated = spec.decorated,
        .fullscreen = spec.fullscreen,
        .vsync = spec.vsync,
        .enableImgui = spec.enableImgui,
    };

    return InitializeWindow(info);
}

}  // namespace cave
