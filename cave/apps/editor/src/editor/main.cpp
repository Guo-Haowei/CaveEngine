#include "engine/core/string/string_utils.h"
#include "engine/drivers/glfw/glfw_display_manager.h"
#include "engine/renderer/graphics_dvars.h"
#include "engine/runtime/framework/EntryPoint.h"
#include "engine/runtime/framework/RuntimeState.h"
#include "engine/runtime/framework/ISceneManager.h"
#include "engine/scripting/lua/lua_script_manager.h"

#include "modules/box2d/box2d_physics_manager.h"
#include "modules/bullet3/bullet3_physics_manager.h"

#include "editor/editor_asset_manager.h"
#include "editor/editor_state.h"
#include "editor/editor_scene_manager.h"
#include "editor/project_browser_state.h"

#define DEFINE_DVAR
#include "editor_dvars.h"
#undef DEFINE_DVAR

namespace cave {

namespace fs = std::filesystem;

void RegisterExtraDvars() {
#define REGISTER_DVAR
#include "editor_dvars.h"
#undef REGISTER_DVAR
}

class Editor : public Application {
public:
    Editor(const ApplicationSpec& p_spec)
        : Application(p_spec, Application::Type::Editor)
        , m_is_world_2d(DVAR_GET_BOOL(is_world_2d)) {
        // m_mode_manager = std::unique_ptr<ModeManager>(new EditorModeManager(*this));
    }

    auto Initialize() -> Result<void> final {
        if (auto res = Application::Initialize(); !res) {
            return res;
        }

        // @TODO: refactor this part
#if 0
        const AppStateId initial_state = AppStateId::Editor;
#else
        const AppStateId initial_state = AppStateId::ProjectBrowser;
#endif
        AppStateMachine::RegisterCreateFunc(AppStateId::ProjectBrowser, [](Application& p_app) {
            auto state = std::make_unique<ProjectBrowserState>(p_app);
            return std::unique_ptr<AppState>(std::move(state));
        });

        AppStateMachine::RegisterCreateFunc(AppStateId::Editor, [](Application& p_app) {
            auto state = std::make_unique<EditorState>(p_app);
            return std::unique_ptr<AppState>(std::move(state));
        });

        AppStateMachine::RegisterCreateFunc(AppStateId::Runtime, [](Application& p_app) {
            auto state = std::make_unique<RuntimeState>(p_app);
            return std::unique_ptr<AppState>(std::move(state));
        });

        m_state_machine = std::make_unique<AppStateMachine>(*this);
        m_state_machine->Init(initial_state);
        return Result<void>();
    }

    void Finalize() override {
        if (m_display_server) {
            [[maybe_unused]] auto [w, h] = m_display_server->GetWindowSize();
            DVAR_SET_IVEC2(window_resolution, w, h);
        }

        Application::Finalize();
    }

    bool IsWorld2D() const override {
        return m_is_world_2d;
    }

private:
    const bool m_is_world_2d;
};

Application* CreateApplication() {
    std::string_view root = StringUtils::BasePath(__FILE__);
    root = StringUtils::BasePath(root);
    root = StringUtils::BasePath(root);

    // @TODO: virtual fs and mount
    auto user_path = fs::path{ root } / "user";
    auto user_string = user_path.string();

    ApplicationSpec spec{};
    spec.userFolder = user_string;
    spec.name = "Editor";
    spec.backend = Backend::EMPTY;
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

}  // namespace cave

int main(int p_argc, const char** p_argv) {
    using namespace cave;

    IAssetManager::RegisterCreateFunc([]() -> IAssetManager* {
        return new EditorAssetManager();
    });
    ISceneManager::RegisterCreateFunc([]() -> ISceneManager* {
        return new EditorSceneManager();
    });
    IScriptManager::RegisterCreateFunc([]() -> IScriptManager* {
        return new LuaScriptManager();
    });
    IDisplayManager::RegisterCreateFunc([]() -> IDisplayManager* {
        return new GlfwDisplayManager();
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
