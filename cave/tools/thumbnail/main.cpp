#include "engine/private/core/string/string_utils.h"
#include "engine/private/empty/empty_display_manager.h"
#include "cave/runtime/framework/IApplication.h"
#include "engine/private/runtime/framework/EntryPoint.h"

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

extern IApplication* CreateCliApp(const ApplicationSpec& p_spec);

IApplication* CreateApp() {
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
    return CreateCliApp(spec);
}

void DestroyApp(IApplication* p_app) {
    if (DEV_VERIFY(p_app)) {
        delete p_app;
    }
}

}  // namespace cave

int main(int p_argc, const char** p_argv) {
    using namespace cave;

    IDisplayManager::RegisterCreateFunc([]() -> IDisplayManager* {
        return new EmptyDisplayManager();
    });
    IGraphicsManager::RegisterCreateFunc([]() -> IGraphicsManager* {
        return new SwGraphicsManager();
    });

    return Main(p_argc, p_argv);
}
