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
#include "engine/private/runtime/dvar/DvarCache.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/BootLoadPipeline.h"
#include "engine/private/runtime/framework/CommonDvars.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/framework/ServiceRegistry.h"
#include "engine/private/runtime/framework/IPhysicsManager.h"
#include "engine/private/runtime/framework/TaskManager.h"
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/projects/ProjectManager.h"
#include "engine/private/runtime/view/ViewManager.h"

// @TODO: remove
#include "engine/private/renderer/graphics_dvars.h"

#if USING(PLATFORM_WASM)
static cave::IApplication* s_app = nullptr;
#endif

namespace cave {

#if USING(USE_COMMAND)
extern void registerCommands(CommandRegistry& cmd_reg);
#endif

namespace fs = std::filesystem;

Application::Application(const AppSpec& p_spec, AppType p_type)
    : IApplication(p_spec)
    , type_(p_type)
    , state_machine_(*this) {

    // @TODO: refactor this select work directory
    vfs_.Mount("@user", fs::path(m_spec.userFolder));
}

IApplication::~IApplication() = default;
Application::~Application() = default;

void Application::RegisterModule(IService* p_module) {
    DEV_ASSERT(p_module);
    p_module->SetApp(this);
    subsystems_.push_back(p_module);
}

Result<ImguiManager*> Application::CreateImguiManager() {
    return new ImguiManager();
}

auto Application::SetupModules() -> Result<void> {
    m_cmd_reg = new cave::CommandRegistry();
    m_console = new cave::Console(*this);

    asset_manager_ = CreateAssetService();
    asset_registry_ = new AssetRegistry();
    m_physics_manager = CreatePhysicsService();
    render_device_ = CreateRenderDevice(m_spec.backend);
    display_service_ = CreateDisplayService();
    input_service_ = new cave::InputService();
    task_manager_ = new TaskManager();

    // @TODO: dependency injection?
    renderer_ = std::make_unique<render::Renderer>(*render_device_);

    scene_scheduler_ = std::make_unique<SceneScheduler>(
        scene_registry_,
        *m_script_service);

    scene_query_ = std::make_unique<SceneQueryService>(scene_registry_);

    view_manager_ = std::make_unique<ViewManager>(scene_registry_,
                                                  render_device_->backend() == rhi::Backend::OpenGL);

    project_manager_ = std::make_unique<ProjectManager>(vfs_,
                                                        *task_manager_,
                                                        *asset_manager_,
                                                        *asset_registry_,
                                                        *renderer_);
    ui_ = std::make_unique<UIRuntime>(*view_manager_);

    // setup app services
    services_.asset_manager_ = asset_manager_;
    services_.asset_registry_ = asset_registry_;
    services_.display_service_ = display_service_;
    services_.input_service_ = input_service_;
    services_.intent_dispatcher_ = &intent_dispatcher_;
    services_.ui_ = ui_.get();
    services_.project_manager_ = project_manager_.get();
    services_.scene_query_ = scene_query_.get();
    services_.scene_registry_ = &scene_registry_;
    services_.scene_scheduler_ = scene_scheduler_.get();
    services_.task_manager_ = task_manager_;
    services_.view_manager_ = view_manager_.get();
    services_.vfs_ = &vfs_;
    services_.render_device_ = render_device_;
    services_.renderer_ = renderer_.get();

    // register subsystems
    RegisterModule(task_manager_);
    RegisterModule(asset_manager_);
    RegisterModule(asset_registry_);
    RegisterModule(m_script_service);
    RegisterModule(m_physics_manager);
    RegisterModule(input_service_);
    RegisterModule(display_service_);
    RegisterModule(render_device_);

    if (m_spec.enableImgui) {
        auto res = CreateImguiManager();
        if (!res) {
            return CAVE_ERROR(res.error());
        }
        m_imgui_manager = *res;
        RegisterModule(m_imgui_manager);
    }

    event_queue_.RegisterListener(render_device_);

    // @TODO: move to registerCommands
    DvarCache::registerCmd(*m_cmd_reg);

    registerCommands(*m_cmd_reg);
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

    for (IService* module : subsystems_) {
        stopwatch_.Restart();
        if (auto res = module->Initialize(); !res) {
            LOG_ERROR("Error: failed to initialize module '{}'", module->GetName());
            return CAVE_ERROR(res.error());
        }
        stopwatch_.Stop();
        LOG_INFO(LogChannel::App, "+{} {}", module->GetName(), stopwatch_.Elapsed().ToString());
    }

    stopwatch_.Restart();
    return Result<void>();
}

void Application::Finalize() {
    state_machine_.shutdown();

    // @TODO: move it to request shutdown
    thread::RequestShutdown();

    for (int index = (int)subsystems_.size() - 1; index >= 0; --index) {
        IService* module = subsystems_[index];
        module->Finalize();
        LOG_TRACE(LogChannel::App, "-{}", module->GetName());
        // @TODO: use smart pointer
        delete module;
    }
}

float Application::UpdateTime() {
    const Nanoseconds elapsed = stopwatch_.Restart();
    const float elapsed_sec = static_cast<float>(elapsed.ToSeconds());

    return math::min(elapsed_sec, 0.5f);
}

bool Application::MainLoop() {
    using namespace render;
    CAVE_PROFILE_FRAME("MainThread");

    OS::singleton().logger().flush();

    display_service_->beginFrame();
    if (display_service_->shouldClose()) {
        return false;
    }

    task_manager_->TickMainThread();

    FrameTime time{
        .dt = UpdateTime(),
        .frame_index = frame_counter_++,
    };

    input_service_->tick(time);

    ui_->beginFrame(input_service_->getUIInput());

    asset_manager_->Update();

    // layer should set active scene
    // update layers from back to front
    view_manager_->beginFrame();

    state_machine_.tick(time);
    intent_dispatcher_.flush();

    // update scene after ImGui, physics and script updates
    scene_scheduler_->tick(time);

    ui_->endFrame();

    std::span<const ResolvedView> views = view_manager_->endFrame();
    renderer_->tick(time, views, ui_->takeDrawData());

    return true;
}

// @TODO: get rid of this
void IApplication::Run(IApplication* p_app) {
    LOG_INFO(LogChannel::App, "----------- Enter Main Loop -----------");

#if USING(PLATFORM_WASM)
    s_app = p_app;
    emscripten_set_main_loop([]() {
        s_app->MainLoop();
    },
                             -1, 1);
#else
    while (p_app->MainLoop());
#endif

    LOG_INFO(LogChannel::App, "----------- Exit Main Loop -----------");
}

AppStateId Application::GetStateId() const {
    return state_machine_.stateId();
}

// void Application::RequestProject(std::string_view p_path) {
//     DEV_ASSERT(!p_path.empty());
//     DEV_ASSERT_MSG(!m_vfs.HasMount("@res"), "resource folder already mounted");
//
//     fs::path resource_folder = fs::path(p_path) / "resources";
//     m_vfs.Mount("@res", resource_folder);
//
//     fs::path project_setting = fs::path(p_path) / "project.yaml";
//
//     std::ifstream file(project_setting.string());
//     if (file.is_open()) {
//         // @TODO: load stuff
//     }
//
//     m_boot_load_pipeline->RequestProject(resource_folder);
// }

}  // namespace cave
