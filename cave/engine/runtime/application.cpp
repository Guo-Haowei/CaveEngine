#include "application.h"

#include <fstream>
#include <imgui/imgui.h>

#include "engine/debugger/profiler.h"
#include "engine/core/io/file_access.h"
#include "engine/core/os/threads.h"
#include "engine/core/string/string_utils.h"
#include "engine/renderer/graphics_dvars.h"
#include "engine/renderer/graphics_manager.h"
#include "engine/runtime/asset_manager_interface.h"
#include "engine/runtime/asset_registry.h"
#include "engine/runtime/common_dvars.h"
#include "engine/runtime/display_manager.h"
#include "engine/runtime/imgui_manager.h"
#include "engine/runtime/input_manager.h"
#include "engine/runtime/layer.h"
#include "engine/runtime/mode_manager.h"
#include "engine/runtime/module_registry.h"
#include "engine/runtime/render_system.h"
#include "engine/runtime/scene_manager_interface.h"
#include "engine/runtime/script_manager.h"
#include "engine/scene/scene.h"

#if USING(PLATFORM_WASM)
static cave::Application* s_app = nullptr;
#endif

namespace cave {

namespace fs = std::filesystem;

Application::Application(const ApplicationSpec& p_spec, Type p_type)
    : m_type(p_type)
    , m_specification(p_spec) {
    // select work directory
    m_user_folder = std::string{ m_specification.userFolder };

    FileAccess::SetUserFolderCallback([&]() { return m_user_folder.c_str(); });
}

Application::~Application() {
}

ModeManager& Application::GetModeManager() {
    return *m_mode_manager.get();
}

void Application::AttachLayer(Layer* p_layer) {
    DEV_ASSERT(p_layer);

    p_layer->m_app = this;
    p_layer->OnAttach();
    m_layers.emplace_back(p_layer);
}

void Application::DetachLayer(Layer* p_layer) {
    DEV_ASSERT(p_layer);

    auto it = std::find(m_layers.begin(), m_layers.end(), p_layer);
    if (it == m_layers.end()) {
        LOG_WARN("Layer '{}' not found");
        return;
    }

    m_layers.erase(it);
    p_layer->OnDetach();
    p_layer->m_app = nullptr;
}

void Application::RegisterModule(Module* p_module) {
    DEV_ASSERT(p_module);
    p_module->m_app = this;
    m_modules.push_back(p_module);
}

Result<ImguiManager*> Application::CreateImguiManager() {
    return new ImguiManager();
}

auto Application::SetupModules() -> Result<void> {
    // @TODO: configure so it's easier for user to override
    m_asset_manager = CreateAssetManager();
    m_asset_registry = new AssetRegistry();
    m_script_manager = CreateScriptManager();
    m_scene_manager = CreateSceneManager();
    m_physics_manager = CreatePhysicsManager();
    m_graphics_manager = CreateGraphicsManager();
    m_display_server = CreateDisplayManager();
    m_input_manager = new InputManager();
    m_render_system = new RenderSystem();

    RegisterModule(m_asset_manager);
    RegisterModule(m_asset_registry);
    RegisterModule(m_scene_manager);
    RegisterModule(m_script_manager);
    RegisterModule(m_physics_manager);
    RegisterModule(m_display_server);
    RegisterModule(m_graphics_manager);
    RegisterModule(m_render_system);
    RegisterModule(m_input_manager);

    if (m_specification.enableImgui) {
        auto res = CreateImguiManager();
        if (!res) {
            return CAVE_ERROR(res.error());
        }
        m_imgui_manager = *res;
        RegisterModule(m_imgui_manager);
    }

    m_event_queue.RegisterListener(m_graphics_manager);

    return Result<void>();
}

auto Application::Initialize() -> Result<void> {
    // @TODO: refactor this part
    {
        m_project_folder = DVAR_GET_STRING(project);
        DEV_ASSERT(!m_project_folder.empty());
        fs::path resource_folder = fs::path(m_project_folder) / "resources";
        m_resource_folder = resource_folder.string();

        FileAccess::SetResFolderCallback([&]() { return m_resource_folder.c_str(); });

        fs::path project_setting = fs::path(m_project_folder) / "project.yaml";

        std::ifstream file(project_setting.string());
        if (file.is_open()) {
        }
    }

    // select backend
    {
        const std::string& backend = DVAR_GET_STRING(gfx_backend);
        if (!backend.empty()) {
            do {
#define BACKEND_DECLARE(ENUM, STR, DVAR)         \
    if (backend == #DVAR) {                      \
        m_specification.backend = Backend::ENUM; \
        break;                                   \
    }
                BACKEND_LIST
#undef BACKEND_DECLARE
                return CAVE_ERROR(ErrorCode::ERR_INVALID_PARAMETER, "Unkown backend '{}', set to 'empty'", backend);
            } while (0);
        }
    }

    if (auto res = SetupModules(); !res) {
        return CAVE_ERROR(res.error());
    }

    for (Module* module : m_modules) {
        LOG("module '{}' being initialized...", module->GetName());
        if (auto res = module->Initialize(); !res) {
            // LOG_ERROR("Error: failed to initialize module '{}'", module->GetName());
            return CAVE_ERROR(res.error());
        }
        LOG("module '{}' initialized\n", module->GetName());
    }

    InitLayers();
    for (auto& layer : m_layers) {
        layer->m_app = this;
        layer->OnAttach();
        LOG("[Runtime] layer '{}' attached!", layer->GetName());
    }

    return Result<void>();
}

void Application::Finalize() {
    for (auto& layer : m_layers) {
        layer->OnDetach();
        LOG("[Runtime] layer '{}' detached!", layer->GetName());
    }
    m_layers.clear();

    // @TODO: move it to request shutdown
    thread::RequestShutdown();

    for (int index = (int)m_modules.size() - 1; index >= 0; --index) {
        Module* module = m_modules[index];
        module->Finalize();
        LOG_VERBOSE("module '{}' finalized", module->GetName());
    }
}

float Application::UpdateTime() {
    float timestep = static_cast<float>(m_timer.GetDuration().ToSecond());
    m_timer.Start();
    return min(timestep, 0.5f);
}

bool Application::MainLoop() {
    CAVE_PROFILE_FRAME("MainThread");

    // === Begin Frame ===
    m_display_server->BeginFrame();
    if (m_display_server->ShouldClose()) {
        return false;
    }

    m_render_system->BeginFrame();
    m_input_manager->BeginFrame();

    // === Update Phase ===
    const float timestep = UpdateTime();

    /*
    @TODO: refactor this to update in the following order
    1. Input System            (poll input, dispatch events)
    2. Script System           (Lua or custom scripts modify components)
    3. Animation System        (update animation timers & apply output to transforms, visuals)
    4. Transformation System   (update local-to-world matrices, resolve hierarchy)
    5. Physics System          (simulate rigidbodies, detect collisions)
    6. Late Script Callbacks   (optional scripts react to post-physics state)
    7. Rendering Prep          (culling, batching, sorting)
    8. Render System           (submit to GPU)
    */

    m_scene_manager->Update();

    // layer should set active scene
    // update layers from back to front
    for (int i = (int)m_layers.size() - 1; i >= 0; --i) {
        m_layers[i]->OnUpdate(timestep);
    }

    // @TODO: refactor this
    if (m_imgui_manager) {
        {
            CAVE_PROFILE_EVENT("ImGuiManager::BeginFrame");
            m_imgui_manager->BeginFrame();
        }

        for (int i = (int)m_layers.size() - 1; i >= 0; --i) {
            m_layers[i]->OnImGuiRender();
        }

        {
            CAVE_PROFILE_EVENT("ImGui::Render");
            ImGui::Render();
        }
    }

    const GameMode game_mode = m_mode_manager->GetMode();
    // change game mode from here

    // @TODO: set mode here
    std::shared_ptr<Scene> scene = m_scene_manager->GetActiveScene();

    if (scene && game_mode == GameMode::Gameplay) {
        m_script_manager->Update(*scene, timestep);
    }

    if (scene) {
        scene->Update(timestep);
    }

    // @TODO: register system instead of if else
    if (scene && game_mode == GameMode::Gameplay) {
        m_physics_manager->Update(*scene, timestep);
    }

    m_render_system->RenderFrame(scene.get());

    // === Rendering Phase ===
    m_graphics_manager->Update(scene.get());

    // === End Frame ===
    m_input_manager->EndFrame();
    return true;
}

void Application::Run(Application* p_app) {
    LOG("\n********************************************************************************"
        "\nMain Loop"
        "\n********************************************************************************");

#if USING(PLATFORM_WASM)
    s_app = p_app;
    emscripten_set_main_loop([]() {
        s_app->MainLoop();
    },
                             -1, 1);
#else
    while (p_app->MainLoop());
#endif

    LOG("\n********************************************************************************"
        "\nMain Loop"
        "\n********************************************************************************");
}

GameLayer* Application::GetGameLayer() {
    return m_game_layer.get();
}

void Application::AttachGameLayer() {
    if (m_game_layer) {
        AttachLayer(m_game_layer.get());
    }
}

void Application::DetachGameLayer() {
    if (m_game_layer) {
        DetachLayer(m_game_layer.get());
    }
}

}  // namespace cave
