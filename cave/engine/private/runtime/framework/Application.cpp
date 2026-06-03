#include "Application.h"

#include <fstream>
#include <imgui/imgui.h>

#include "cave/core/diagnostics/Profiler.h"
#include "cave/core/string/StringUtils.h"
#include "cave/core/time/FrameTime.h"
#include "cave/runtime/intent/IntentDispatcher.h"

#include "engine/private/core/diagnostics/console/Console.h"
#include "engine/private/core/diagnostics/log_sink/CompositeLogger.h"
#include "engine/private/core/io/file_access.h"
#include "engine/private/core/os/threads.h"
#include "engine/private/render/renderer/Renderer.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/runtime/dvar/DvarCache.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/CommonDvars.h"
#include "engine/private/runtime/display/DisplayService.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/framework/ServiceRegistry.h"
#include "engine/private/runtime/framework/IPhysicsManager.h"
#include "engine/private/runtime/framework/IScriptService.h"
#include "engine/private/runtime/framework/TaskManager.h"
#include "engine/private/runtime/view/ViewManager.h"
#include "engine/private/runtime/scene/SceneQueryService.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/ui/UIRuntime.h"

// @TODO: remove
#include "engine/private/renderer/graphics_dvars.h"

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
    m_vfs.Mount("@user", fs::path(m_spec.userFolder));
}

IApplication::~IApplication() = default;
Application::~Application() = default;

void Application::RegisterModule(IService* p_module) {
    DEV_ASSERT(p_module);
    p_module->SetApp(this);
    m_modules.push_back(p_module);
}

Result<ImguiManager*> Application::CreateImguiManager() {
    return new ImguiManager();
}

auto Application::SetupModules() -> Result<void> {
    m_cmd_reg = new cave::CommandRegistry();
    m_console = new cave::Console(*this);

    m_asset_manager = CreateAssetService();
    m_asset_registry = new AssetRegistry();
    m_script_service = CreateScriptService();
    m_scene_registry = new SceneRegistry();
    m_physics_manager = CreatePhysicsService();
    m_render_device = CreateRenderDevice(m_spec.backend);
    m_display_service = CreateDisplayService();
    m_input_service = new cave::InputService();
    m_ui = new UIRuntime();
    m_renderer = new render::Renderer();
    m_view_manager = new ViewManager();
    m_task_manager = new TaskManager();
    m_intent_dispatcher = new cave::IntentDispatcher();

    m_boot_load_pipeline = std::make_unique<BootLoadPipeline>(
        *m_task_manager,
        *m_asset_manager,
        *m_asset_registry);

    m_scene_scheduler = std::make_unique<SceneScheduler>(
        *m_scene_registry,
        *m_script_service);

    m_scene_query_service = new cave::SceneQueryService(*m_scene_registry);

    RegisterModule(m_task_manager);
    RegisterModule(m_asset_manager);
    RegisterModule(m_asset_registry);
    RegisterModule(m_scene_registry);
    RegisterModule(m_script_service);
    RegisterModule(m_physics_manager);
    RegisterModule(m_input_service);
    RegisterModule(m_display_service);
    RegisterModule(m_render_device);
    RegisterModule(m_renderer);
    RegisterModule(m_view_manager);
    RegisterModule(m_ui);
    RegisterModule(m_intent_dispatcher);

    if (m_spec.enableImgui) {
        auto res = CreateImguiManager();
        if (!res) {
            return CAVE_ERROR(res.error());
        }
        m_imgui_manager = *res;
        RegisterModule(m_imgui_manager);
    }

    m_event_queue.RegisterListener(m_render_device);

    DvarCache::RegisterCmd(*m_cmd_reg);
    return Result<void>();
}

auto Application::Initialize() -> Result<void> {
    LOG_WARN("@TODO: move thumbnail render target creation to somewhere else");
    LOG_WARN("@TODO: support material in path tracer");
    LOG_WARN("@TODO: remove global path tracer object");
    LOG_WARN("@TODO: accumulate path tracer result");
    LOG_WARN("@TODO: NotifyPropertyChanged, for edit, undo, redo");

    // select backend
    {
        const std::string& backend = DVAR_GET_STRING(gfx_backend);
        if (!backend.empty()) {
            do {
#define BACKEND_DECLARE(ENUM, STR, DVAR)     \
    if (backend == #DVAR) {                  \
        m_spec.backend = rhi::Backend::ENUM; \
        break;                               \
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

    for (IService* module : m_modules) {
        m_stopwatch.Restart();
        if (auto res = module->Initialize(); !res) {
            LOG_ERROR("Error: failed to initialize module '{}'", module->GetName());
            return CAVE_ERROR(res.error());
        }
        m_stopwatch.Stop();
        LOG_OK(LogChannel::App, "+{} {}", module->GetName(), m_stopwatch.Elapsed().ToString());
    }

    m_stopwatch.Restart();
    return Result<void>();
}

void Application::Finalize() {
    m_state_machine.Shutdown();

    // @TODO: move it to request shutdown
    thread::RequestShutdown();

    for (int index = (int)m_modules.size() - 1; index >= 0; --index) {
        IService* module = m_modules[index];
        module->Finalize();
        LOG_TRACE(LogChannel::App, "-{}", module->GetName());
        // @TODO: use smart pointer
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

    m_display_service->BeginFrame();
    if (m_display_service->ShouldClose()) {
        return false;
    }

    m_task_manager->TickMainThread();

    FrameTime time{
        .dt = UpdateTime(),
        .frame_index = m_frame_counter++,
    };

    m_input_service->Tick(time);

    m_ui->BeginFrame(m_input_service->GetUIInput());

    m_asset_manager->Update();

    // layer should set active scene
    // update layers from back to front
    m_view_manager->BeginFrame();

    m_state_machine.Tick(time);
    m_intent_dispatcher->Flush();

    // update scene after ImGui, physics and script updates
    m_scene_scheduler->Tick(time);

    m_ui->EndFrame();

    std::span<const ResolvedView> views = m_view_manager->EndFrame();
    auto ui_draw_data = m_ui->TakeDrawData();
    m_renderer->Tick(time, views, ui_draw_data);

    return true;
}

// @TODO: get rid of this
void IApplication::Run(IApplication* p_app) {
    LOG_INFO("----------- Enter Main Loop -----------");

#if USING(PLATFORM_WASM)
    s_app = p_app;
    emscripten_set_main_loop([]() {
        s_app->MainLoop();
    },
                             -1, 1);
#else
    while (p_app->MainLoop());
#endif

    LOG_INFO("----------- Exit Main Loop -----------");
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
