#include "application.h"

#include <fstream>
#include <imgui/imgui.h>

#include "engine/core/debugger/profiler.h"
#include "engine/core/dynamic_variable/dynamic_variable_manager.h"
#include "engine/core/io/file_access.h"
#include "engine/core/os/threads.h"
#include "engine/core/string/string_utils.h"
#include "engine/renderer/graphics_dvars.h"
#include "engine/renderer/graphics_manager.h"
#include "engine/runtime/asset_manager.h"
#include "engine/runtime/asset_registry.h"
#include "engine/runtime/common_dvars.h"
#include "engine/runtime/display_manager.h"
#include "engine/runtime/imgui_manager.h"
#include "engine/runtime/input_manager.h"
#include "engine/runtime/layer.h"
#include "engine/runtime/module_registry.h"
#include "engine/runtime/render_system.h"
#include "engine/runtime/scene_manager_interface.h"
#include "engine/runtime/script_manager.h"
#include "engine/scene/scene.h"

#define DEFINE_DVAR
#include "engine/renderer/graphics_dvars.h"
#undef DEFINE_DVAR
#define DEFINE_DVAR
#include "engine/runtime/common_dvars.h"
#undef DEFINE_DVAR

#if USING(PLATFORM_WASM)
static cave::Application* s_app = nullptr;
#endif

namespace cave {

namespace fs = std::filesystem;

// @TODO: refactor
[[maybe_unused]] static constexpr const char* DVAR_CACHE_FILE = "@user://dynamic_variables.cache";

static void RegisterCommonDvars() {
#define REGISTER_DVAR
#include "engine/runtime/common_dvars.h"
#undef REGISTER_DVAR
}

static void RegisterRenderDvars() {
#define REGISTER_DVAR
#include "engine/renderer/graphics_dvars.h"
#undef REGISTER_DVAR
}

Application::Application(const ApplicationSpec& p_spec, Type p_type)
    : m_specification(p_spec), m_type(p_type) {
    // select work directory
    m_userFolder = std::string{ m_specification.userFolder };

    FileAccess::SetUserFolderCallback([&]() { return m_userFolder.c_str(); });
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

void Application::SaveCommandLine(int p_argc, const char** p_argv) {
    m_appName = p_argv[0];
    for (int i = 1; i < p_argc; ++i) {
        m_commandLine.push_back(p_argv[i]);
    }
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
    m_asset_manager = new AssetManager();
    m_asset_registry = new AssetRegistry();
    m_script_manager = CreateScriptManager();
    m_scene_manager = CreateSceneManager();
    m_physics_manager = CreatePhysicsManager();
    m_graphics_manager = CreateGraphicsManager();
    m_display_server = DisplayManager::Create();
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

void Application::RegisterDvars() {
    RegisterCommonDvars();
    RegisterRenderDvars();
}

auto Application::Initialize(int p_argc, const char** p_argv) -> Result<void> {
    SaveCommandLine(p_argc, p_argv);

    RegisterDvars();
#if USING(ENABLE_DVAR)
    DynamicVariableManager::Deserialize(DVAR_CACHE_FILE);
    // parse happens after deserialization, so command line will override cache
    DynamicVariableManager::Parse(m_commandLine);
#endif

    // @TODO: refactor this part
    {
        m_projectFolder = DVAR_GET_STRING(project);
        fs::path resource_folder = fs::path(m_projectFolder) / "resources";
        m_resourceFolder = resource_folder.string();

        FileAccess::SetResFolderCallback([&]() { return m_resourceFolder.c_str(); });

        fs::path project_setting = fs::path(m_projectFolder) / "project.yaml";

        std::ifstream file(project_setting.string());
        if (file.is_open()) {
        }
    }

    // select window size
    {
        const Vector2i resolution{ DVAR_GET_IVEC2(window_resolution) };
        const Vector2i max_size{ 3840, 2160 };  // 4K
        const Vector2i min_size{ 480, 360 };    // 360P
        Vector2i desired_size;
        if (resolution.x > 0 && resolution.y > 0) {
            desired_size = resolution;
        } else {
            desired_size = Vector2i(m_specification.width, m_specification.height);
        }
        desired_size = clamp(desired_size, min_size, max_size);
        m_specification.width = desired_size.x;
        m_specification.height = desired_size.y;
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
    // @TODO: fix
    if (m_display_server) {
        auto [w, h] = m_display_server->GetWindowSize();
        DVAR_SET_IVEC2(window_resolution, w, h);
    }

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

#if USING(ENABLE_DVAR)
    DynamicVariableManager::Serialize(DVAR_CACHE_FILE);
#endif
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

    // 1. scene manager checks for update, and if it's necessary to swap scene
    // 2. renderer builds render data
    // 3. editor modifies scene
    // 4. script manager updates logic
    // 5. phyiscs manager updates physics
    // 6. graphcs manager renders (optional: on another thread)

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
        CAVE_PROFILE_EVENT("Application::ImGui");
        m_imgui_manager->BeginFrame();

        for (int i = (int)m_layers.size() - 1; i >= 0; --i) {
            m_layers[i]->OnImGuiRender();
        }

        ImGui::Render();
    }

    Scene* scene = m_scene_manager->GetActiveScene();

    if (scene) {
        m_script_manager->Update(*scene, timestep);
        scene->Update(timestep);
    }

    m_render_system->RenderFrame(scene);

    if (scene && m_state == State::SIM) {
        m_physics_manager->Update(*scene, timestep);
    }

    // === Rendering Phase ===
    m_graphics_manager->Update(scene);

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

void Application::AttachGameLayer() {
    if (m_gameLayer) {
        AttachLayer(m_gameLayer.get());
    }
}

void Application::DetachGameLayer() {
    if (m_gameLayer) {
        DetachLayer(m_gameLayer.get());
    }
}

// @TODO: refactor this
void Application::SetState(State p_state) {
    switch (p_state) {
        case State::BEGIN_SIM: {
            if (DEV_VERIFY(m_state == State::EDITING)) {
                m_state = p_state;
            }
        } break;
        case State::END_SIM: {
            if (DEV_VERIFY(m_state == State::SIM)) {
                m_state = p_state;
            }
        } break;
        case State::SIM: {
            if (DEV_VERIFY(m_state == State::BEGIN_SIM)) {
                m_state = p_state;
            }
        } break;
        case State::EDITING: {
            if (DEV_VERIFY(m_state == State::END_SIM)) {
                m_state = p_state;
            }
        } break;
        default:
            CRASH_NOW();
            break;
    }
}

}  // namespace cave
