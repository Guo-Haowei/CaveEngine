#include "GlfwDisplayManager.h"

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
#include "engine/private/runtime/framework/ImGuiManager.h"

#if USING(PLATFORM_WINDOWS)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

namespace cave {

using rhi::Backend;

auto GlfwDisplayManager::InitializeWindow(const WindowSpecfication& p_spec) -> Result<void> {
    m_backend = p_spec.backend;
    m_title = p_spec.title;

    glfwSetErrorCallback([](int code, const char* desc) { LOG_FATAL("[glfw] error({}): {}", code, desc); });

    // @TODO: resizable
    // @TODO: fullscreen
    glfwInit();

    glfwWindowHint(GLFW_DECORATED, p_spec.decorated);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);

    switch (m_backend) {
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

    m_window = glfwCreateWindow(p_spec.width,
                                p_spec.height,
                                p_spec.title.c_str(),
                                nullptr, nullptr);
    DEV_ASSERT(m_window);

    glfwSetWindowSizeCallback(m_window, WindowSizeCallback);

    glfwSetWindowCloseCallback(m_window, [](GLFWwindow* p_window) {
        glfwSetWindowShouldClose(p_window, GLFW_FALSE);
        QuitVote vote = DisplayService::GetSingleton().GetApp()->OnQuitRequested({ QuitReason::WindowClose });
        switch (vote) {
            case QuitVote::Allow: {
                glfwSetWindowShouldClose(p_window, true);
            } break;
            case QuitVote::Deny: {
                glfwSetWindowShouldClose(p_window, false);
            } break;
        }
    });

    InputService& input = m_app->InputService();
    {
        InputDeviceId kb_id = InputDeviceId::NextId();
        auto keyboard_mouse_device = std::make_unique<GlfwKeyboardMouseDevice>(kb_id);
        keyboard_mouse_device->InstallCallbacks(m_window);
        input.addDevice(std::move(keyboard_mouse_device));
    }
    {
        InputDeviceId pad_id = InputDeviceId::NextId();
        auto keyboard_mouse_device = std::make_unique<GlfwGamepadDevice>(pad_id, GLFW_JOYSTICK_1);
        input.addDevice(std::move(keyboard_mouse_device));

        // input.Router;
    }

    glfwSetWindowPos(m_window, 200, 200);
    glfwGetWindowSize(m_window, &m_frameSize.x, &m_frameSize.y);

    switch (m_backend) {
        case Backend::OpenGL:
            glfwMakeContextCurrent(m_window);
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
            return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "backend '{}' not supported by glfw", (int)m_backend);
    }

    auto imgui = m_app->GetImguiManager();
    if (imgui) {
        imgui->SetDisplayCallbacks(
            [this]() {
                switch (m_backend) {
                    case Backend::OpenGL:
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

void GlfwDisplayManager::FinalizeImpl() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool GlfwDisplayManager::ShouldClose() {
    return glfwWindowShouldClose(m_window);
}

void GlfwDisplayManager::BeginFrame() {
    glfwPollEvents();
    glfwGetWindowPos(m_window, &m_windowPos.x, &m_windowPos.y);
}

std::string_view GlfwDisplayManager::GetTitle() {
    return glfwGetWindowTitle(m_window);
}

void GlfwDisplayManager::SetTitle(std::string_view p_title) {
    glfwSetWindowTitle(m_window, p_title.data());
}

void* GlfwDisplayManager::GetNativeWindow() {
#if USING(PLATFORM_WINDOWS)
    return glfwGetWin32Window(m_window);
#else
    return nullptr;
#endif
}

std::tuple<int, int> GlfwDisplayManager::GetWindowSize() { return std::tuple<int, int>(m_frameSize.x, m_frameSize.y); }

std::tuple<int, int> GlfwDisplayManager::GetWindowPos() { return std::tuple<int, int>(m_windowPos.x, m_windowPos.y); }

void GlfwDisplayManager::WindowSizeCallback(GLFWwindow*, int p_width, int p_height) {
    GlfwDisplayManager& window = reinterpret_cast<GlfwDisplayManager&>(DisplayService::GetSingleton());

    auto event = std::make_shared<ResizeEvent>(p_width, p_height);
    window.m_frameSize.x = p_width;
    window.m_frameSize.y = p_height;
    window.m_app->GetEventQueue().DispatchEvent(event);
}

}  // namespace cave

// @TODO: get rid of this, discusting af
WARNING_DISABLE(4127, "-Wunused-parameter")
WARNING_DISABLE(4189, "-Wunused-parameter")
#include <imgui/backends/imgui_impl_glfw.cpp>
