#include <filesystem>

#include "editor/editor_layer.h"
#include "editor/editor_scene_manager.h"
#include "engine/core/string/string_utils.h"
#include "engine/renderer/graphics_dvars.h"
#include "engine/runtime/entry_point.h"
#include "engine/runtime/layer.h"
#include "engine/runtime/mode_manager.h"
#include "engine/runtime/scene_manager_interface.h"
#include "modules/box2d/box2d_physics_manager.h"
#include "modules/bullet3/bullet3_physics_manager.h"

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

class EditorModeManager : public ModeManager {
public:
    EditorModeManager(Application& p_app)
        : ModeManager(GameMode::Editor, p_app) {}

    void SetMode(GameMode p_mode) {
        LOG("attempt to transit from mode {} to {}", (int)m_mode, (int)p_mode);
        if (p_mode == m_mode) {
            return;
        }

        auto& scene_manager = reinterpret_cast<EditorSceneManager&>(ISceneManager::GetSingleton());
        GameLayer* game_layer = m_app.GetGameLayer();
        DEV_ASSERT(game_layer);
        switch (p_mode) {
            case GameMode::Editor: {
                m_app.DetachGameLayer();
                game_layer->SetActiveScene(nullptr);
                scene_manager.CloseSimScene();
            } break;
            case GameMode::Gameplay: {
                std::shared_ptr<Scene> sim_scene = std::make_shared<Scene>();
                {
                    std::shared_ptr<Scene> current_scene = scene_manager.GetActiveScene();
                    sim_scene->Copy(*current_scene);
                    sim_scene->Update(0.0f);
                }

                scene_manager.OpenSimScene(sim_scene);
                game_layer->SetActiveScene(std::move(sim_scene));
                m_app.AttachGameLayer();
            } break;
            case GameMode::CutScene:
            case GameMode::Loading:
            case GameMode::Paused:
            default:
                CRASH_NOW_MSG("mode not supported");
                break;
        }

        m_mode = p_mode;
    }
};

class Editor : public Application {
public:
    Editor(const ApplicationSpec& p_spec)
        : Application(p_spec, Application::Type::Editor) {
        m_mode_manager = std::unique_ptr<ModeManager>(new EditorModeManager(*this));
    }

    void InitLayers() override {
        m_editorLayer = std::make_unique<EditorLayer>();
        AttachLayer(m_editorLayer.get());

        // Only creates game layer, don't attach yet
        m_game_layer = std::make_unique<GameLayer>("GameLayer");
    }

    CameraComponent* GetActiveCamera() override {
        return m_editorLayer->GetActiveCamera();
    }

private:
    std::unique_ptr<EditorLayer> m_editorLayer;
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
    spec.width = 800;
    spec.height = 600;
    spec.backend = Backend::EMPTY;
    spec.decorated = true;
    spec.fullscreen = false;
    spec.vsync = false;
    spec.enableImgui = true;
    return new Editor(spec);
}

}  // namespace cave

int main(int p_argc, const char** p_argv) {
    using namespace cave;

    // @TODO: figure out a way to create it cleanly
#if !USING(PLATFORM_WASM)
#if 0
    IPhysicsManager::RegisterCreateFunc([]() -> IPhysicsManager* {
        return new Bullet3PhysicsManager();
    });
#else
    IPhysicsManager::RegisterCreateFunc([]() -> IPhysicsManager* {
        return new Box2dPhysicsManager();
    });
#endif
#endif
    ISceneManager::RegisterCreateFunc([]() -> ISceneManager* {
        return new EditorSceneManager();
    });

    return Main(p_argc, p_argv);
}
