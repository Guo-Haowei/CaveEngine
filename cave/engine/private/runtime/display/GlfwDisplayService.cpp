#include "GlfwDisplayService.h"

#include <GLFW/glfw3.h>
#include <imgui/backends/imgui_impl_glfw.h>

#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/render/renderer/GraphicsDvars.h"
#include "engine/private/runtime/framework/CommonDvars.h"
#include "engine/private/runtime/framework/EventQueue.h"
#include "engine/private/runtime/input/GlfwGamepadDevice.h"
#include "engine/private/runtime/input/GlfwKeyboardMouseDevice.h"
#include "engine/private/runtime/input/InputService.h"

#include "modules/opengl_common/OpenGlDefines.h"

// @TODO: refactor
// do not put ImGui code here
#include "engine/private/runtime/framework/ImGuiManager.h"

#if USING(PLATFORM_WINDOWS)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

namespace cave {

GlfwDisplayService::GlfwDisplayService()
    : DisplayService("GlfwDisplayService") {}

auto GlfwDisplayService::initializeWindow(const WindowSpecfication& spec) -> Result<void> {
    using render::Backend;

    m_backend = spec.backend;
    m_title = spec.title;

    glfwSetErrorCallback([](int code, const char* desc) { LOG_FATAL("[glfw] error({}): {}", code, desc); });

    // @TODO: resizable
    // @TODO: fullscreen
    glfwInit();

    glfwWindowHint(GLFW_DECORATED, spec.decorated);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);

    switch (m_backend) {
        case Backend::OpenGl:
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, CAVE_GL_VERSION_MAJOR);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, CAVE_GL_VERSION_MINOR);
            if (DVAR_GET_BOOL(gfx_gpu_validation)) {
                glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
            }
            break;
        default:
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            break;
    }

    m_window = glfwCreateWindow(spec.width,
                                spec.height,
                                spec.title.c_str(),
                                nullptr, nullptr);
    DEV_ASSERT(m_window);

    glfwSetWindowSizeCallback(m_window, windowSizeCallback);

    glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
        glfwSetWindowShouldClose(window, GLFW_FALSE);
        QuitVote vote = DisplayService::singleton().GetApp()->onQuitRequested({ QuitReason::WindowClose });
        switch (vote) {
            case QuitVote::Allow: {
                glfwSetWindowShouldClose(window, true);
            } break;
            case QuitVote::Deny: {
                glfwSetWindowShouldClose(window, false);
            } break;
        }
    });

    InputService& input = m_app->services().inputService();
    {
        InputDeviceId kb_id = InputDeviceId::nextId();
        auto keyboard_mouse_device = MakeOwner<GlfwKeyboardMouseDevice>(kb_id);
        keyboard_mouse_device->InstallCallbacks(m_window);
        input.addDevice(std::move(keyboard_mouse_device));
    }
    {
        InputDeviceId pad_id = InputDeviceId::nextId();
        auto keyboard_mouse_device = MakeOwner<GlfwGamepadDevice>(pad_id, GLFW_JOYSTICK_1);
        input.addDevice(std::move(keyboard_mouse_device));
    }

    glfwSetWindowPos(m_window, 40, 40);
    glfwGetWindowSize(m_window, &m_frame_size.x, &m_frame_size.y);

    switch (m_backend) {
        case Backend::OpenGl:
            glfwMakeContextCurrent(m_window);
            break;
        case Backend::Vulkan:
            if (!glfwVulkanSupported()) {
                return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Vulkan not supported");
            }
            break;
        case Backend::Metal:
        case Backend::D3d11:
        case Backend::D3d12:
            break;
        default:
            return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "backend '{}' not supported by glfw", (int)m_backend);
    }

    if (ImGuiService* imgui = m_app->services().imgui) {
        imgui->setDisplayCallbacks(
            [this]() {
                switch (m_backend) {
                    case Backend::OpenGl:
                        ImGui_ImplGlfw_InitForOpenGL(m_window, false);
                        break;
                    case Backend::Vulkan:
                        ImGui_ImplGlfw_InitForVulkan(m_window, false);
                        break;
                    default:
                        ImGui_ImplGlfw_InitForOther(m_window, false);
                        break;
                }

                glfwSetWindowFocusCallback(m_window, ImGui_ImplGlfw_WindowFocusCallback);
                glfwSetCursorEnterCallback(m_window, ImGui_ImplGlfw_CursorEnterCallback);
                glfwSetCharCallback(m_window, ImGui_ImplGlfw_CharCallback);
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
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool GlfwDisplayService::shouldClose() {
    return glfwWindowShouldClose(m_window);
}

void GlfwDisplayService::beginFrame() {
    glfwPollEvents();
    int x, y;
    glfwGetWindowPos(m_window, &x, &y);
    m_window_pos.x = (float)x;
    m_window_pos.y = (float)y;
}

std::string_view GlfwDisplayService::title() {
    return glfwGetWindowTitle(m_window);
}

void GlfwDisplayService::title(std::string_view p_title) {
    glfwSetWindowTitle(m_window, p_title.data());
}

void* GlfwDisplayService::nativeWindow() {
#if USING(PLATFORM_WINDOWS)
    return glfwGetWin32Window(m_window);
#else
    CRASH_NOW();
    return nullptr;
#endif
}

void GlfwDisplayService::windowSizeCallback(GLFWwindow*, int w, int h) {
    GlfwDisplayService& window = reinterpret_cast<GlfwDisplayService&>(DisplayService::singleton());

    window.m_frame_size.x = w;
    window.m_frame_size.y = h;

    ResizeEvent event(w, h);

    window.m_app->eventQueue().dispatchEvent(&event);
}

}  // namespace cave

WARNING_DISABLE(4127, "-Wunused-parameter")
WARNING_DISABLE(4189, "-Wunused-parameter")
#include <imgui/backends/imgui_impl_glfw.cpp>
