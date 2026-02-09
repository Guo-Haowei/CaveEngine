#include "DisplayService.h"

#include "engine/private/drivers/glfw/glfw_display_manager.h"
#include "cave/runtime/framework/IApplication.h"
#include "engine/private/runtime/framework/CommonDvars.h"
#include "engine/private/renderer/graphics_dvars.h"

namespace cave {

using rhi::Backend;

Result<void> DisplayService::InitializeImpl() {
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
