#include "glfw_display_manager.h"

#include <GLFW/glfw3.h>
#include <imgui/backends/imgui_impl_glfw.h>

#include "engine/private/debugger/profiler.h"
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/runtime/framework/Application.h"
#include "engine/private/runtime/framework/CommonDvars.h"
#include "engine/private/runtime/framework/InputSystem.h"

#include "engine/private/drivers/glfw/glfw_gamepad_device.h"
#include "engine/private/drivers/glfw/glfw_keyboard_mouse_device.h"

// @TODO: refactor
#include "engine/private/runtime/framework/ImGuiManager.h"

#if USING(PLATFORM_WINDOWS)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

namespace cave {

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
        case Backend::OPENGL:
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

    InputSystem& input = *m_app->GetInputSystem();
    {
        InputDeviceId kb_id = InputDeviceId::NextId();
        auto keyboard_mouse_device = std::make_unique<GlfwKeyboardMouseDevice>(kb_id);
        keyboard_mouse_device->InstallCallbacks(m_window);
        input.AddDevice(std::move(keyboard_mouse_device));
    }
    {
        InputDeviceId pad_id = InputDeviceId::NextId();
        auto keyboard_mouse_device = std::make_unique<GlfwGamepadDevice>(pad_id, GLFW_JOYSTICK_1);
        input.AddDevice(std::move(keyboard_mouse_device));

        // input.Router;
    }

    glfwSetWindowPos(m_window, 200, 200);
    glfwGetWindowSize(m_window, &m_frameSize.x, &m_frameSize.y);

    switch (m_backend) {
        case Backend::OPENGL:
            glfwMakeContextCurrent(m_window);
            break;
        case Backend::VULKAN:
            if (!glfwVulkanSupported()) {
                return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Vulkan not supported");
            }
            break;
        case Backend::METAL:
        case Backend::D3D11:
        case Backend::D3D12:
            break;
        default:
            return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "backend '{}' not supported by glfw", ToString(m_backend));
    }

    auto imgui = m_app->GetImguiManager();
    if (imgui) {
        imgui->SetDisplayCallbacks(
            [this]() {
                switch (m_backend) {
                    case Backend::OPENGL:
                        ImGui_ImplGlfw_InitForOpenGL(m_window, false);
                        break;
                    case Backend::VULKAN:
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

void GlfwDisplayManager::WindowSizeCallback(GLFWwindow* p_window, int p_width, int p_height) {
    auto window = reinterpret_cast<GlfwDisplayManager*>(glfwGetWindowUserPointer(p_window));

    auto event = std::make_shared<ResizeEvent>(p_width, p_height);
    window->m_frameSize.x = p_width;
    window->m_frameSize.y = p_height;
    window->m_app->GetEventQueue().DispatchEvent(event);
}

}  // namespace cave

// @TODO: get rid of this, discusting af
WARNING_DISABLE(4127, "-Wunused-parameter")
WARNING_DISABLE(4189, "-Wunused-parameter")
#include <imgui/backends/imgui_impl_glfw.cpp>
