#include <filesystem>

#include "editor/editor_layer.h"
#include "engine/core/string/string_utils.h"
#include "engine/renderer/graphics_dvars.h"
#include "engine/runtime/entry_point.h"
#include "engine/runtime/scene_manager_interface.h"
#include "modules/bullet3/bullet3_physics_manager.h"

#define DEFINE_DVAR
#include "editor_dvars.h"
#undef DEFINE_DVAR

namespace my {

namespace fs = std::filesystem;

extern Scene* CreateTheAviatorScene();
extern Scene* CreateBoxScene();
extern Scene* CreatePbrTestScene();
extern Scene* CreatePhysicsTestScene();

// @TODO: EditorContext

class Editor : public Application {
public:
    Editor(const ApplicationSpec& p_spec) : Application(p_spec, Application::Type::EDITOR) {}

    void InitLayers() override {
        m_editorLayer = std::make_unique<EditorLayer>();
        AttachLayer(m_editorLayer.get());

        // Only creates game layer, don't attach yet
        m_gameLayer = std::make_unique<GameLayer>("GameLayer");
    }

    CameraComponent* GetActiveCamera() override {
        return &m_editorLayer->GetActiveCamera();
    }

private:
    void RegisterDvars() override;

    std::unique_ptr<EditorLayer> m_editorLayer;
};

void Editor::RegisterDvars() {
    Application::RegisterDvars();
#define REGISTER_DVAR
#include "editor_dvars.h"
#undef REGISTER_DVAR
}

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

class EditorSceneManager : public ISceneManager {
public:
    EditorSceneManager()
        : ISceneManager("EditorSceneManager") {}

    auto InitializeImpl() -> Result<void> override;

    void FinalizeImpl() override {}

    Scene* GetActiveScene() override;

    void Update() override;

protected:
    Scene* m_active_scene = nullptr;
};

auto EditorSceneManager::InitializeImpl() -> Result<void> {
    auto scene = DVAR_GET_STRING(default_scene);
    if (scene == "pbr_test") {
        m_active_scene = CreatePbrTestScene();
    } else if (scene == "physics_test") {
        m_active_scene = CreatePhysicsTestScene();
    } else if (scene == "the_aviator") {
        m_active_scene = CreateTheAviatorScene();
    } else if (scene == "box") {
        m_active_scene = CreateBoxScene();
    }

    return Result<void>();
}

Scene* EditorSceneManager::GetActiveScene() {
    return m_active_scene;
}

void EditorSceneManager::Update() {
}

}  // namespace my

#if 0
Scene* Application::CreateInitialScene() {
    ecs::Entity::SetSeed();

    Scene* scene = new Scene;

    Vector2i frame_size = DVAR_GET_IVEC2(resolution);

    auto root = scene->CreateTransformEntity("world");
    scene->m_root = root;

#if 0
    {

        // clang-format off
        const std::vector<std::vector<int>> data = {
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
            { 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, },
            { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, },
            { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
            { 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
            { 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
            { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, },
            { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, },
            { 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, },
            { 1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
        };
        // clang-format on

        // test code, remember to take out
        auto id = scene->CreateTileMapEntity("tile_map");
        scene->AttachChild(id);

        TileMapComponent* tileMap = scene->GetComponent<TileMapComponent>(id);
        tileMap->FromArray(data);

        auto& sprite = tileMap->m_sprite;

        auto res = (m_assetRegistry->FindByPath("@res://images/tiles.png")).value().Wait<ImageAsset>();

        sprite.texture = (*res).get();

        const int grid_x = 3;
        const int grid_y = 2;

        const float dx = 1.0f / grid_x;
        const float dy = 1.0f / grid_y;

        for (int y = 0; y < grid_y; ++y) {
            for (int x = 0; x < grid_x; ++x) {
                const float u0 = x * dx;
                const float v0 = (y + 1) * dy;
                const float u1 = (x + 1) * dx;
                const float v1 = y * dy;

                sprite.frames.push_back(Rect(Vector2f(u0, v0), Vector2f(u1, v1)));
            }
        }
    }
#endif

    // test code, remember to take out

    return scene;
}
#endif

int main(int p_argc, const char** p_argv) {
    using namespace my;

#if !USING(PLATFORM_WASM)
    IPhysicsManager::RegisterCreateFunc([]() -> IPhysicsManager* {
        return new Bullet3PhysicsManager();
    });
#endif
    ISceneManager::RegisterCreateFunc([]() -> ISceneManager* {
        return new EditorSceneManager();
    });

    return Main(p_argc, p_argv);
}
