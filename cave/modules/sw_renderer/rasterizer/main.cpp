#include "engine/core/string/string_utils.h"
#include "engine/drivers/windows/win32_display_manager.h"
#include "engine/null/null_graphics_manager.h"
#include "engine/runtime/application.h"
#include "engine/runtime/entry_point.h"
#include "engine/runtime/mode_manager.h"

namespace cave {

namespace fs = std::filesystem;

void RegisterExtraDvars() {
}

class SwRendererApp : public Application {
public:
    SwRendererApp(const ApplicationSpec& p_spec)
        : Application(p_spec, Application::Type::Tool) {
        m_mode_manager = std::unique_ptr<ModeManager>(new ModeManager(GameMode::Tool, *this));
    }

    CameraComponent* GetActiveCamera() override {
        return nullptr;
    }

    bool IsWorld2D() const override {
        return false;
    }
};

Application* CreateApplication() {
    // @TODO: get rid of this
    std::string_view root = StringUtils::BasePath(__FILE__);
    root = StringUtils::BasePath(root);
    root = StringUtils::BasePath(root);

    auto user_path = fs::path{ root } / "user";
    auto user_string = user_path.string();

    ApplicationSpec spec{};
    spec.userFolder = user_string;
    spec.name = "SoftwareRenderer";
    spec.width = 256;
    spec.height = 256;
    spec.backend = Backend::EMPTY;
    spec.decorated = true;
    spec.fullscreen = false;
    spec.vsync = false;
    spec.enableImgui = false;
    return new SwRendererApp(spec);
}

}  // namespace cave

int main(int p_argc, const char** p_argv) {
    using namespace cave;

    IDisplayManager::RegisterCreateFunc([]() -> IDisplayManager* {
        return new Win32DisplayManager();
    });
    IGraphicsManager::RegisterCreateFunc([]() -> IGraphicsManager* {
        return new NullGraphicsManager();
    });

    return Main(p_argc, p_argv);
}
