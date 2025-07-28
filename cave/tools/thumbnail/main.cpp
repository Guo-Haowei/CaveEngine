#include "engine/core/string/string_utils.h"
#include "engine/drivers/windows/win32_display_manager.h"
#include "engine/empty/empty_display_manager.h"
#include "engine/runtime/application.h"
#include "engine/runtime/entry_point.h"

#include "modules/sw/sw_renderer.h"

#define DEFINE_DVAR
#include "thumbnail_dvars.h"
#undef DEFINE_DVAR

namespace cave {

namespace fs = std::filesystem;

void RegisterExtraDvars() {
#define REGISTER_DVAR
#include "thumbnail_dvars.h"
#undef REGISTER_DVAR
}

extern Application* CreateGuiApp(const ApplicationSpec& p_spec);
extern Application* CreateCliApp(const ApplicationSpec& p_spec);

Application* CreateApplication() {
    // @TODO: get rid of this
    std::string_view root = StringUtils::BasePath(__FILE__);
    root = StringUtils::BasePath(root);
    root = StringUtils::BasePath(root);

    auto user_path = fs::path{ root } / "user";
    auto user_string = user_path.string();

    int dim = DVAR_GET_INT(thumbnail_size);
    dim = clamp(dim, 64, 1024);
    DVAR_SET_INT(thumbnail_size, dim);

    ApplicationSpec spec{};
    spec.userFolder = user_string;
    spec.name = "SoftwareRenderer";
    spec.width = dim;
    spec.height = dim;
    spec.backend = Backend::EMPTY;
    spec.decorated = true;
    spec.fullscreen = false;
    spec.vsync = false;
    spec.enableImgui = false;
    if (DVAR_GET_BOOL(headless)) {
        return CreateCliApp(spec);
    } else {
        return CreateGuiApp(spec);
    }
}

}  // namespace cave

int main(int p_argc, const char** p_argv) {
    using namespace cave;

    // @TODO: check if headless

    IDisplayManager::RegisterCreateFunc([]() -> IDisplayManager* {
        if (DVAR_GET_BOOL(headless)) {
            return new EmptyDisplayManager();
        } else {
            return new Win32DisplayManager();
        }
    });
    IGraphicsManager::RegisterCreateFunc([]() -> IGraphicsManager* {
        return new SwGraphicsManager();
    });

    return Main(p_argc, p_argv);
}
