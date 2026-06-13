#include "cave/core/string/StringUtils.h"

#include "engine/private/runtime/display/GlfwDisplayService.h"
#include "engine/private/runtime/framework/Application.h"
#include "engine/private/runtime/framework/EntryPoint.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/script/lua/LuaScriptService.h"

#include "modules/box2d/box2d_physics_manager.h"
#include "modules/bullet3/bullet3_physics_manager.h"

#include "editor/EditorAssetManager.h"
#include "editor/EditorState.h"
#include "editor/ProjectBrowserState.h"
#include "editor/services/Workspace.h"

#define DEFINE_DVAR
#include "EditorDvars.h"
#undef DEFINE_DVAR

namespace cave {

namespace fs = std::filesystem;
using namespace math;

void RegisterExtraDvars() {
#define REGISTER_DVAR
#include "EditorDvars.h"
#undef REGISTER_DVAR
}

class Editor final : public Application {
public:
    Editor(const AppSpec& p_spec)
        : Application(p_spec, AppType::Editor)
        , m_is_world_2d(DVAR_GET_BOOL(is_world_2d)) {
    }

    Result<void> Initialize() final {
        if (auto res = Application::Initialize(); !res) {
            return res;
        }

        // @TODO: refactor this part
#if 0
        const AppStateId initial_state = AppStateId::Editor;
#else
        const AppStateId initial_state = AppStateId::ProjectBrowser;
#endif
        AppStateMachine::registerCreateFunc(AppStateId::ProjectBrowser, [](IApplication& p_app) {
            auto state = std::make_unique<ProjectBrowserState>(p_app);
            return std::unique_ptr<AppState>(std::move(state));
        });

        AppStateMachine::registerCreateFunc(AppStateId::Editor, [](IApplication& p_app) {
            auto state = std::make_unique<EditorState>(p_app);
            return std::unique_ptr<AppState>(std::move(state));
        });

        m_state_machine.initialize(initial_state);
        return Result<void>();
    }

    void Finalize() final {
        if (display_service_) {
            [[maybe_unused]] auto window_size = display_service_->windowSize();
            DVAR_SET_IVEC2(window_resolution, window_size.x, window_size.y);
        }

        Application::Finalize();
    }

    QuitVote OnQuitRequested(const QuitContext&) override {
        if (EditorState* editor = dynamic_cast<EditorState*>(m_state_machine.appState())) {
            const bool should_quit = editor->services().workspace().onCloseRequested();
            if (!should_quit) {
                return QuitVote::Deny;
            }
        }

        return QuitVote::Allow;
    }

private:
    const bool m_is_world_2d;
};

IApplication* CreateApp() {
    std::string_view root = StringUtils::BasePath(__FILE__);
    root = StringUtils::BasePath(root);
    root = StringUtils::BasePath(root);

    // @TODO: virtual fs and mount
    auto user_path = fs::path{ root } / "user";
    auto user_string = user_path.string();

    AppSpec spec{};
    spec.userFolder = user_string;
    spec.name = "Editor";
    spec.backend = rhi::Backend::Null;
    spec.decorated = true;
    spec.fullscreen = false;
    spec.vsync = false;
    spec.enableImgui = true;

    // window size
    {
        const Vector2i resolution{ DVAR_GET_IVEC2(window_resolution) };
        const Vector2i max_size{ 3840, 2160 };  // 4K
        const Vector2i min_size{ 480, 360 };    // 360p
        Vector2i desired_size;
        if (resolution.x > 0 && resolution.y > 0) {
            desired_size = resolution;
        } else {
            desired_size = Vector2i(spec.width, spec.height);
        }
        desired_size = clamp(desired_size, min_size, max_size);
        spec.width = desired_size.x;
        spec.height = desired_size.y;
    }

    return new Editor(spec);
}

void DestroyApp(IApplication* p_app) {
    if (DEV_VERIFY(p_app)) {
        delete p_app;
    }
}

}  // namespace cave

int main(int p_argc, const char** p_argv) {
    using namespace cave;

    IAssetManager::RegisterCreateFunc([]() -> IAssetManager* {
        return new EditorAssetManager();
    });
    IScriptService::RegisterCreateFunc([]() -> IScriptService* {
        return new LuaScriptService();
    });
    DisplayService::RegisterCreateFunc([]() -> DisplayService* {
        return new GlfwDisplayService();
    });

    // @TODO: figure out a way to create it cleanly
#if !USING(PLATFORM_WASM)
    IPhysicsManager::RegisterCreateFunc([]() -> IPhysicsManager* {
        if (DVAR_GET_BOOL(is_world_2d)) {
            return new Box2dPhysicsManager();
        } else {
            return new Bullet3PhysicsManager();
        }
    });
#endif

    return Main(p_argc, p_argv);
}
