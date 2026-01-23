#include "application.h"

#include <fstream>
#include <imgui/imgui.h>

#include "engine/debugger/profiler.h"
#include "engine/core/io/file_access.h"
#include "engine/core/io/logger.h"
#include "engine/core/os/threads.h"
#include "engine/core/string/string_utils.h"
#include "engine/renderer/graphics_dvars.h"
#include "engine/renderer/graphics_manager.h"
#include "engine/runtime/app_state.h"
#include "engine/runtime/asset_manager_interface.h"
#include "engine/runtime/asset_registry.h"
#include "engine/runtime/boot_load_pipeline.h"
#include "engine/runtime/common_dvars.h"
#include "engine/runtime/display_manager.h"
#include "engine/runtime/imgui_manager.h"
#include "engine/runtime/input_manager.h"
#include "engine/runtime/module_registry.h"
#include "engine/runtime/render_system.h"
#include "engine/runtime/scene_manager_interface.h"
#include "engine/runtime/script_manager.h"
#include "engine/runtime/task_manager.h"
#include "engine/runtime/vfs.h"
#include "engine/runtime/viewport_manager.h"
#include "engine/scene/scene.h"

#if USING(PLATFORM_WASM)
static cave::Application* s_app = nullptr;
#endif

namespace cave {

namespace fs = std::filesystem;

Application::Application(const ApplicationSpec& p_spec, Type p_type)
    : m_type(p_type)
    , m_specification(p_spec) {

    m_vfs = std::make_unique<VFS>();

    // @TODO: refactor this select work directory
    m_vfs->Mount("@user", fs::path(m_specification.userFolder));
}

Application::~Application() {
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
    m_viewport_manager = new ViewportManager();
    m_task_manager = new TaskManager();

    m_boot_load_pipeline = std::make_unique<BootLoadPipeline>(
        *m_task_manager,
        *m_asset_manager,
        *m_asset_registry);

    RegisterModule(m_task_manager);
    RegisterModule(m_asset_manager);
    RegisterModule(m_asset_registry);
    RegisterModule(m_scene_manager);
    RegisterModule(m_script_manager);
    RegisterModule(m_physics_manager);
    RegisterModule(m_input_manager);
    RegisterModule(m_display_server);
    RegisterModule(m_graphics_manager);
    RegisterModule(m_render_system);
    RegisterModule(m_viewport_manager);

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
        if (auto res = module->Initialize(); !res) {
            LOG_ERROR("Error: failed to initialize module '{}'", module->GetName());
            return CAVE_ERROR(res.error());
        }
        LOG_OK("module '{}' initialized", module->GetName());
    }

    return Result<void>();
}

void Application::Finalize() {
    m_state_machine->Shutdown();

    // @TODO: move it to request shutdown
    thread::RequestShutdown();

    for (int index = (int)m_modules.size() - 1; index >= 0; --index) {
        Module* module = m_modules[index];
        module->Finalize();
        LOG_VERBOSE("module '{}' finalized", module->GetName());
    }
}

float Application::UpdateTime() {
    const float timestep = static_cast<float>(m_timer.GetDuration().ToSecond());
    m_timer.Start();
    return min(timestep, 0.5f);
}

bool Application::MainLoop() {
    CAVE_PROFILE_FRAME("MainThread");

    CompositeLogger::GetSingleton().Flush();

    // === Begin Frame ===
    m_display_server->BeginFrame();
    if (m_display_server->ShouldClose()) {
        return false;
    }

    m_task_manager->TickMainThread();

    m_render_system->BeginFrame();
    m_input_manager->Update();

    // === Update Phase ===
    const float timestep = UpdateTime();

    m_asset_manager->Update();
    m_scene_manager->Update();

    // layer should set active scene
    // update layers from back to front

    m_state_machine->Tick(timestep);

    std::shared_ptr<Scene> scene = m_scene_manager->GetActiveScene();
    m_viewport_manager->UpdateProviders(timestep);

    if (scene) {
        scene->Update(timestep);
    }

    // view has camera controller and camera manager
    const bool is_opengl = m_graphics_manager->GetBackend() == Backend::OPENGL;

    std::vector<SceneView> views;
    m_viewport_manager->BuildViews(views, is_opengl);

    // @TODO: build render data, rename it to something better
    m_render_system->RenderFrame(views);

    // @TODO: think of how to handle multiple view
    m_graphics_manager->Update();

    // === End Frame ===
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

AppStateId Application::GetStateId() const {
    return m_state_machine->GetStateId();
}

void Application::RequestProject(std::string_view p_path) {
    DEV_ASSERT(!p_path.empty());
    DEV_ASSERT_MSG(!m_vfs->HasMount("@res"), "resource folder already mounted");

    fs::path resource_folder = fs::path(p_path) / "resources";
    m_vfs->Mount("@res", resource_folder);

    fs::path project_setting = fs::path(p_path) / "project.yaml";

    std::ifstream file(project_setting.string());
    if (file.is_open()) {
        // @TODO: load stuff
    }

    m_boot_load_pipeline->RequestProject(resource_folder);
}

BootLoadPipeline& Application::GetBootLoadPipeline() {
    DEV_ASSERT(m_boot_load_pipeline);
    return *m_boot_load_pipeline;
}

}  // namespace cave
