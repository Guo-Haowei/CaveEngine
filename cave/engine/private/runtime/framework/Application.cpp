#include "Application.h"

#include <fstream>
#include <imgui/imgui.h>

#include "engine/private/core/debugger/Profiler.h"
#include "engine/private/core/io/file_access.h"
#include "engine/private/core/logging/Logger.h"
#include "engine/private/core/os/threads.h"
#include "engine/private/render/renderer/Renderer.h"
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/runtime/string/StringUtils.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/CommonDvars.h"
#include "engine/private/runtime/framework/DisplayManager.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/framework/InputSystem.h"
#include "engine/private/runtime/framework/ModuleRegistry.h"
#include "engine/private/runtime/framework/IPhysicsManager.h"
#include "engine/private/runtime/framework/IScriptManager.h"
#include "engine/private/runtime/framework/TaskManager.h"
#include "engine/private/runtime/framework/ViewManager.h"
#include "engine/private/runtime/scene/SceneQueryService.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

#if USING(PLATFORM_WASM)
static cave::IApplication* s_app = nullptr;
#endif

namespace cave {

namespace fs = std::filesystem;

Application::Application(const AppSpec& p_spec, AppType p_type)
    : IApplication(p_spec)
    , m_type(p_type)
    , m_state_machine(*this) {

    // @TODO: refactor this select work directory
    m_vfs.Mount("@user", fs::path(m_specification.userFolder));
}

IApplication::~IApplication() = default;
Application::~Application() = default;

void Application::RegisterModule(Module* p_module) {
    DEV_ASSERT(p_module);
    p_module->SetApp(this);
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
    m_scene_registry = new SceneRegistry();
    m_physics_manager = CreatePhysicsManager();
    m_render_device = CreateRenderDevice();
    m_display_server = CreateDisplayManager();
    m_input_system = new InputSystem();
    m_renderer = new render::Renderer();
    m_view_manager = new ViewManager();
    m_task_manager = new TaskManager();

    m_boot_load_pipeline = std::make_unique<BootLoadPipeline>(
        *m_task_manager,
        *m_asset_manager,
        *m_asset_registry);

    m_scene_scheduler = std::make_unique<SceneScheduler>(
        *m_scene_registry,
        *m_script_manager);

    m_scene_query_service = new cave::SceneQueryService(*m_scene_registry);

    RegisterModule(m_task_manager);
    RegisterModule(m_asset_manager);
    RegisterModule(m_asset_registry);
    RegisterModule(m_scene_registry);
    RegisterModule(m_script_manager);
    RegisterModule(m_physics_manager);
    RegisterModule(m_input_system);
    RegisterModule(m_display_server);
    RegisterModule(m_render_device);
    RegisterModule(m_renderer);
    RegisterModule(m_view_manager);

    if (m_specification.enableImgui) {
        auto res = CreateImguiManager();
        if (!res) {
            return CAVE_ERROR(res.error());
        }
        m_imgui_manager = *res;
        RegisterModule(m_imgui_manager);
    }

    m_event_queue.RegisterListener(m_render_device);

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
        m_stopwatch.Restart();
        if (auto res = module->Initialize(); !res) {
            LOG_ERROR("Error: failed to initialize module '{}'", module->GetName());
            return CAVE_ERROR(res.error());
        }
        m_stopwatch.Stop();
        LOG_OK("module '{}' initialized in {}", module->GetName(), m_stopwatch.Elapsed().ToString());
    }

    m_stopwatch.Restart();
    return Result<void>();
}

void Application::Finalize() {
    m_state_machine.Shutdown();

    // @TODO: move it to request shutdown
    thread::RequestShutdown();

    for (int index = (int)m_modules.size() - 1; index >= 0; --index) {
        Module* module = m_modules[index];
        module->Finalize();
        LOG_VERBOSE("module '{}' finalized", module->GetName());
        delete module;
    }
}

float Application::UpdateTime() {
    const Nanoseconds elapsed = m_stopwatch.Restart();
    const float elapsed_sec = static_cast<float>(elapsed.ToSeconds());

    return math::min(elapsed_sec, 0.5f);
}

bool Application::MainLoop() {
    using namespace render;
    CAVE_PROFILE_FRAME("MainThread");

    CompositeLogger::GetSingleton().Flush();

    // === Begin Frame ===
    m_display_server->BeginFrame();
    if (m_display_server->ShouldClose()) {
        return false;
    }

    m_task_manager->TickMainThread();

    m_input_system->Update();

    // === Update Phase ===
    const float timestep = UpdateTime();

    m_asset_manager->Update();

    // layer should set active scene
    // update layers from back to front
    m_view_manager->BeginFrame();

    m_state_machine.Tick(timestep);

    // update scene after ImGui, physics and script updates
    m_scene_scheduler->Tick(timestep);

    std::span<const ResolvedView> views = m_view_manager->EndFrame();
    m_renderer->Tick(views);

    return true;
}

// @TODO: get rid of this
void IApplication::Run(IApplication* p_app) {
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
    return m_state_machine.GetStateId();
}

void Application::RequestProject(std::string_view p_path) {
    DEV_ASSERT(!p_path.empty());
    DEV_ASSERT_MSG(!m_vfs.HasMount("@res"), "resource folder already mounted");

    fs::path resource_folder = fs::path(p_path) / "resources";
    m_vfs.Mount("@res", resource_folder);

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
