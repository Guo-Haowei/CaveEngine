#include "Application.h"

#include <fstream>
#include <imgui/imgui.h>

#include "cave/core/diagnostics/Profiler.h"
#include "cave/core/string/StringUtils.h"
#include "cave/core/threading/Threads.h"
#include "cave/core/time/FrameTime.h"
#include "cave/runtime/display/DisplayService.h"

#include "engine/private/core/diagnostics/console/Console.h"
#include "engine/private/core/io/file_access.h"
#include "engine/private/core/os/os.h"
#include "engine/private/render/renderer/Renderer.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/BootLoadPipeline.h"
#include "engine/private/runtime/framework/CommonDvars.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/framework/ServiceRegistry.h"
#include "engine/private/runtime/framework/IPhysicsManager.h"
#include "engine/private/runtime/framework/TaskManager.h"
#include "engine/private/runtime/projects/ProjectManager.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/scene/SceneScheduler.h"
#include "engine/private/runtime/view/ViewManager.h"

// @TODO: remove
#include "engine/private/renderer/graphics_dvars.h"

#if USING(PLATFORM_WASM)
static cave::IApplication* s_app = nullptr;
#endif

namespace cave {

#if USING(USE_COMMAND)
extern void RegisterCommands(CommandRegistry& cmd_reg);
#endif

namespace fs = std::filesystem;

Application::Application(const AppSpec& spec, AppType type)
    : IApplication(spec)
    , m_app_type(type)
    , m_state_machine(*this) {

    // @TODO: refactor this select work directory
    m_vfs.Mount("@user", fs::path(m_app_spec.userFolder));
}

Application::~Application() = default;

void Application::registerModule(IService* p_module) {
    DEV_ASSERT(p_module);
    p_module->SetApp(this);
    m_subsystems.push_back(p_module);
}

auto Application::setupModules() -> Result<void> {
    // @TODO: clean up
    m_engine_services.canvas_ = &m_canvas;
    m_engine_services.ui_canvas = &m_ui_canvas;
    m_engine_services.intent_bus = &m_intent_bus;

    m_cmd_reg_ = new cave::CommandRegistry();
    m_console = new cave::Console(*this);

    m_task_manager = new TaskManager();
    m_engine_services.task_manager = m_task_manager;

    m_asset_manager = CreateAssetService();
    m_engine_services.asset_manager = m_asset_manager;

    m_asset_registry = new AssetRegistry();
    m_engine_services.asset_registry = m_asset_registry;

    m_render_device = CreateRenderDevice(m_app_spec.backend);
    m_engine_services.render_device = m_render_device;

    m_display_service = CreateDisplayService();
    m_engine_services.display_service = m_display_service;

    m_input_service = new InputService(m_game_input);
    m_engine_services.input_service = m_input_service;

    m_game_input.setPointer(m_input_service->pointers());

    // @TODO: dependency injection?
    m_scene_registry = MakeOwner<SceneRegistry>();

    m_renderer = MakeOwner<render::Renderer>(m_engine_services);
    m_engine_services.renderer_ = m_renderer.get();

    m_scene_scheduler = MakeOwner<SceneScheduler>(m_engine_services);

    m_view_manager = MakeOwner<ViewManager>(*m_scene_registry,
                                            m_render_device->backend() == rhi::Backend::OpenGL);

    m_project_manager = MakeOwner<ProjectManager>(m_vfs,
                                                  *m_task_manager,
                                                  *m_asset_manager,
                                                  *m_asset_registry,
                                                  *m_renderer);
    m_engine_services.project_manager = m_project_manager.get();

    m_ui_runtime = MakeOwner<UIRuntime>(m_ui_canvas, *m_view_manager);
    m_engine_services.ui = m_ui_runtime.get();

    // setup app services
    m_engine_services.game_input = &m_game_input;
    m_engine_services.native_scripts = &m_native_scripts;
    m_engine_services.scene_registry = m_scene_registry.get();
    m_engine_services.scene_scheduler = m_scene_scheduler.get();
    m_engine_services.view_manager = m_view_manager.get();
    m_engine_services.vfs = &m_vfs;
    m_engine_services.game_module = &m_game_module_handle;

    // register subsystems
    registerModule(m_task_manager);
    registerModule(m_asset_manager);
    registerModule(m_asset_registry);
    registerModule(m_input_service);
    registerModule(m_display_service);
    registerModule(m_render_device);

    if (m_app_spec.enableImgui) {
        m_imgui = new ImGuiService(m_app_spec.backend);
        m_engine_services.imgui = m_imgui;
        registerModule(m_imgui);
    }

    m_event_queue.RegisterListener(m_render_device);

    // @TODO: move to registerCommands
    RegisterCommands(*m_cmd_reg_);
    return Result<void>();
}

auto Application::initialize() -> Result<void> {
    LOG_WARN("@TODO: move thumbnail render target creation to somewhere else");
    LOG_WARN("@TODO: support material in path tracer");
    LOG_WARN("@TODO: remove global path tracer object");
    LOG_WARN("@TODO: accumulate path tracer result");
    LOG_WARN("@TODO: NotifyPropertyChanged, for edit, undo, redo");

    // select backend
    {
        std::string_view backend = DVAR_GET_STRING(gfx_backend);
        if (!backend.empty()) {
            do {
#define BACKEND_DECLARE(ENUM, STR, DVAR)         \
    if (backend == #DVAR) {                      \
        m_app_spec.backend = rhi::Backend::ENUM; \
        break;                                   \
    }
                BACKEND_LIST
#undef BACKEND_DECLARE
                return CAVE_ERROR(ErrorCode::ERR_INVALID_PARAMETER, "Unkown backend '{}', set to 'empty'", backend);
            } while (0);
        }
    }

    if (auto res = setupModules(); !res) {
        return CAVE_ERROR(res.error());
    }

    for (IService* module : m_subsystems) {
        m_stopwatch.Restart();
        if (auto res = module->Initialize(); !res) {
            LOG_ERROR("Error: failed to initialize module '{}'", module->GetName());
            return CAVE_ERROR(res.error());
        }
        m_stopwatch.Stop();
        LOG_INFO(LogChannel::App, "+{} {}", module->GetName(), m_stopwatch.Elapsed().ToString());
    }

    m_stopwatch.Restart();
    return Result<void>();
}

void Application::finalize() {
    m_state_machine.shutdown();

    // @TODO: move it to request shutdown
    thread::RequestShutdown();

    for (int index = (int)m_subsystems.size() - 1; index >= 0; --index) {
        IService* module = m_subsystems[index];
        module->Finalize();
        LOG_TRACE(LogChannel::App, "-{}", module->GetName());
        // @TODO: use smart pointer
        delete module;
    }

    m_scene_registry.reset();
    m_game_module_handle.unload();
}

float Application::updateTime() {
    const Nanoseconds elapsed = m_stopwatch.Restart();
    const float elapsed_sec = static_cast<float>(elapsed.ToSeconds());

    return math::min(elapsed_sec, 0.5f);
}

bool Application::mainLoop() {
    using namespace render;
    CAVE_PROFILE_FRAME("MainThread");

    OS::singleton().logger().flush();

    m_display_service->beginFrame();
    if (m_display_service->shouldClose()) {
        return false;
    }

    m_canvas.beginFrame();
    m_ui_canvas.beginFrame();

    m_task_manager->TickMainThread();

    FrameTime time{
        .dt = updateTime(),
        .frame_index = m_frame_counter++,
    };

    m_input_service->tick(time);

    m_scene_scheduler->flushSceneCommands();

    m_ui_runtime->beginFrame();

    m_asset_manager->update();

    // layer should set active scene
    // update layers from back to front
    m_view_manager->beginFrame();

    m_state_machine.tick(time);  // submit view requests
    m_intent_bus.flush();

    std::span<const ResolvedView> views = m_view_manager->endFrame();

    // update scene after ImGui, physics and script updates
    m_scene_scheduler->tick(time);

    // build UI data
    for (const ResolvedView& view : views) {
        m_ui_runtime->buildDrawList(view);
    }

    m_ui_runtime->endFrame(m_input_service->getUIInput());
    m_canvas.endFrame();
    m_ui_canvas.endFrame();

    m_renderer->tick(time, views);

    return true;
}

// @TODO: get rid of this
void IApplication::run(IApplication* p_app) {
    LOG_INFO(LogChannel::App, "----------- Enter Main Loop -----------");

#if USING(PLATFORM_WASM)
    s_app = p_app;
    emscripten_set_main_loop([]() {
        s_app->MainLoop();
    },
                             -1, 1);
#else
    while (p_app->mainLoop());
#endif

    LOG_INFO(LogChannel::App, "----------- Exit Main Loop -----------");
}

AppStateId Application::stateId() const {
    return m_state_machine.stateId();
}

}  // namespace cave
