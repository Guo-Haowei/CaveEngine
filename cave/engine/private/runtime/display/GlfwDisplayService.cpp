#include "GlfwDisplayService.h"

#include <GLFW/glfw3.h>
#include <imgui/backends/imgui_impl_glfw.h>

#include "cave/core/diagnostics/Log.h"
#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/runtime/framework/CommonDvars.h"
#include "engine/private/runtime/framework/EventQueue.h"
#include "engine/private/runtime/input/GlfwGamepadDevice.h"
#include "engine/private/runtime/input/GlfwKeyboardMouseDevice.h"
#include "engine/private/runtime/input/InputService.h"

// @TODO: refactor
// do not put ImGui code here
#include "engine/private/runtime/framework/ImGuiManager.h"

#if USING(PLATFORM_WINDOWS)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

namespace cave {

auto GlfwDisplayService::initializeWindow(const WindowSpecfication& spec) -> Result<void> {
    using rhi::Backend;

    backend_ = spec.backend;
    title_ = spec.title;

    glfwSetErrorCallback([](int code, const char* desc) { LOG_FATAL("[glfw] error({}): {}", code, desc); });

    // @TODO: resizable
    // @TODO: fullscreen
    glfwInit();

    glfwWindowHint(GLFW_DECORATED, spec.decorated);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);

    switch (backend_) {
        case Backend::OpenGL:
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            if (DVAR_GET_BOOL(gfx_gpu_validation)) {
                glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
            }
            break;
        default:
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            break;
    }

    window_ = glfwCreateWindow(spec.width,
                               spec.height,
                               spec.title.c_str(),
                               nullptr, nullptr);
    DEV_ASSERT(window_);

    glfwSetWindowSizeCallback(window_, windowSizeCallback);

    glfwSetWindowCloseCallback(window_, [](GLFWwindow* window) {
        glfwSetWindowShouldClose(window, GLFW_FALSE);
        QuitVote vote = DisplayService::GetSingleton().GetApp()->OnQuitRequested({ QuitReason::WindowClose });
        switch (vote) {
            case QuitVote::Allow: {
                glfwSetWindowShouldClose(window, true);
            } break;
            case QuitVote::Deny: {
                glfwSetWindowShouldClose(window, false);
            } break;
        }
    });

    InputService& input = m_app->InputService();
    {
        InputDeviceId kb_id = InputDeviceId::nextId();
        auto keyboard_mouse_device = std::make_unique<GlfwKeyboardMouseDevice>(kb_id);
        keyboard_mouse_device->InstallCallbacks(window_);
        input.addDevice(std::move(keyboard_mouse_device));
    }
    {
        InputDeviceId pad_id = InputDeviceId::nextId();
        auto keyboard_mouse_device = std::make_unique<GlfwGamepadDevice>(pad_id, GLFW_JOYSTICK_1);
        input.addDevice(std::move(keyboard_mouse_device));
    }

    glfwSetWindowPos(window_, 200, 200);
    glfwGetWindowSize(window_, &frame_size_.x, &frame_size_.y);

    switch (backend_) {
        case Backend::OpenGL:
            glfwMakeContextCurrent(window_);
            break;
        case Backend::Vulkan:
            if (!glfwVulkanSupported()) {
                return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Vulkan not supported");
            }
            break;
        case Backend::Metal:
        case Backend::Direct3D11:
        case Backend::Direct3D12:
            break;
        default:
            return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "backend '{}' not supported by glfw", (int)backend_);
    }

    auto imgui = m_app->GetImguiManager();
    if (imgui) {
        imgui->SetDisplayCallbacks(
            [this]() {
                switch (backend_) {
                    case Backend::OpenGL:
                        ImGui_ImplGlfw_InitForOpenGL(window_, false);
                        break;
                    case Backend::Vulkan:
                        ImGui_ImplGlfw_InitForVulkan(window_, false);
                        break;
                    default:
                        ImGui_ImplGlfw_InitForOther(window_, false);
                        break;
                }

                glfwSetWindowFocusCallback(window_, ImGui_ImplGlfw_WindowFocusCallback);
                glfwSetCursorEnterCallback(window_, ImGui_ImplGlfw_CursorEnterCallback);
                glfwSetCharCallback(window_, ImGui_ImplGlfw_CharCallback);
            },
            []() {
                ImGui_ImplGlfw_Shutdown();
            },
            []() {
                ImGui_ImplGlfw_NewFrame();
            });
    }

    return Result<void>();
}

void GlfwDisplayService::FinalizeImpl() {
    glfwDestroyWindow(window_);
    glfwTerminate();
}

bool GlfwDisplayService::shouldClose() {
    return glfwWindowShouldClose(window_);
}

void GlfwDisplayService::beginFrame() {
    glfwPollEvents();
    int x, y;
    glfwGetWindowPos(window_, &x, &y);
    window_pos_.x = (float)x;
    window_pos_.y = (float)y;
}

std::string_view GlfwDisplayService::title() {
    return glfwGetWindowTitle(window_);
}

void GlfwDisplayService::title(std::string_view p_title) {
    glfwSetWindowTitle(window_, p_title.data());
}

void* GlfwDisplayService::nativeWindow() {
#if USING(PLATFORM_WINDOWS)
    return glfwGetWin32Window(window_);
#else
    CRASH_NOW();
    return nullptr;
#endif
}

void GlfwDisplayService::windowSizeCallback(GLFWwindow*, int w, int h) {
    GlfwDisplayService& window = reinterpret_cast<GlfwDisplayService&>(DisplayService::GetSingleton());

    auto event = std::make_shared<ResizeEvent>(w, h);
    window.frame_size_.x = w;
    window.frame_size_.y = h;
    window.m_app->GetEventQueue().DispatchEvent(event);
}

}  // namespace cave

WARNING_DISABLE(4127, "-Wunused-parameter")
WARNING_DISABLE(4189, "-Wunused-parameter")
#include <imgui/backends/imgui_impl_glfw.cpp>
