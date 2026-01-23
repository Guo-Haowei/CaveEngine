#include "glfw_display_manager.h"

#include <GLFW/glfw3.h>
#include <imgui/backends/imgui_impl_glfw.h>

#include "engine/debugger/profiler.h"
#include "engine/renderer/graphics_dvars.h"
#include "engine/runtime/application.h"
#include "engine/runtime/common_dvars.h"
#include "engine/runtime/imgui_manager.h"
#include "engine/runtime/input_manager.h"

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

    glfwSetWindowUserPointer(m_window, this);
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

    glfwSetCursorPosCallback(m_window, CursorPosCallback);
    glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
    glfwSetKeyCallback(m_window, KeyCallback);
    glfwSetScrollCallback(m_window, ScrollCallback);
    glfwSetWindowSizeCallback(m_window, WindowSizeCallback);

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

void GlfwDisplayManager::CursorPosCallback(GLFWwindow* p_window, double p_x, double p_y) {
    auto window = reinterpret_cast<GlfwDisplayManager*>(glfwGetWindowUserPointer(p_window));
    //auto input_manager = window->m_app->GetInputManager();

    if (window->m_app->GetSpecification().enableImgui) {
        ImGui_ImplGlfw_CursorPosCallback(p_window, p_x, p_y);
    }

    // if (!ImGui::GetIO().WantCaptureMouse)
    {
        // input_manager->SetCursor(static_cast<float>(p_x), static_cast<float>(p_y));
    }
}

void GlfwDisplayManager::MouseButtonCallback(GLFWwindow* p_window,
                                             int p_button,
                                             int p_action,
                                             int p_mods) {
    auto window = reinterpret_cast<GlfwDisplayManager*>(glfwGetWindowUserPointer(p_window));
    //auto input_manager = window->m_app->GetInputManager();

    if (window->m_app->GetSpecification().enableImgui) {
        ImGui_ImplGlfw_MouseButtonCallback(p_window, p_button, p_action, p_mods);
    }

    // if (!ImGui::GetIO().WantCaptureMouse)
    //{
    //    if (p_action == GLFW_RELEASE) {
    //        input_manager->SetButton(static_cast<MouseButton>(p_button), false);
    //    } else {
    //        input_manager->SetButton(static_cast<MouseButton>(p_button), true);
    //    }
    //}
}

void GlfwDisplayManager::KeyCallback(GLFWwindow* p_window,
                                     int p_keycode,
                                     int p_scancode,
                                     int p_action,
                                     int p_mods) {
    auto window = reinterpret_cast<GlfwDisplayManager*>(glfwGetWindowUserPointer(p_window));
    //auto input_manager = window->m_app->GetInputManager();

    if (window->m_app->GetSpecification().enableImgui) {
        ImGui_ImplGlfw_KeyCallback(p_window, p_keycode, p_scancode, p_action, p_mods);
    }

    //auto& keyMapping = window->m_keyMapping;

    // if (!ImGui::GetIO().WantCaptureKeyboard)
    //{
    //    DEV_ASSERT(keyMapping.find(p_keycode) != keyMapping.end());
    //    Key key = keyMapping[p_keycode];

    //    if (p_action == GLFW_PRESS) {
    //        input_manager->SetKey(key, true);
    //    } else if (p_action == GLFW_RELEASE) {
    //        input_manager->SetKey(key, false);
    //    }
    //}
}

void GlfwDisplayManager::ScrollCallback(GLFWwindow* p_window,
                                        double p_xoffset,
                                        double p_yoffset) {
    auto window = reinterpret_cast<GlfwDisplayManager*>(glfwGetWindowUserPointer(p_window));
    //auto input_manager = window->m_app->GetInputManager();

    if (window->m_app->GetSpecification().enableImgui) {
        ImGui_ImplGlfw_ScrollCallback(p_window, p_xoffset, p_yoffset);
    }

    // if (!ImGui::GetIO().WantCaptureMouse)
    //{
    //    input_manager->SetWheel(p_xoffset, p_yoffset);
    //}
}

void GlfwDisplayManager::WindowSizeCallback(GLFWwindow* p_window, int p_width, int p_height) {
    auto window = reinterpret_cast<GlfwDisplayManager*>(glfwGetWindowUserPointer(p_window));

    auto event = std::make_shared<ResizeEvent>(p_width, p_height);
    window->m_frameSize.x = p_width;
    window->m_frameSize.y = p_height;
    window->m_app->GetEventQueue().DispatchEvent(event);
}

void GlfwDisplayManager::InitializeKeyMapping() {
    if (!m_keyMapping.empty()) {
        return;
    }

    m_keyMapping[GLFW_KEY_SPACE] = Key::KEY_SPACE;
    m_keyMapping[GLFW_KEY_APOSTROPHE] = Key::KEY_APOSTROPHE;
    m_keyMapping[GLFW_KEY_COMMA] = Key::KEY_COMMA;
    m_keyMapping[GLFW_KEY_MINUS] = Key::KEY_MINUS;
    m_keyMapping[GLFW_KEY_PERIOD] = Key::KEY_PERIOD;
    m_keyMapping[GLFW_KEY_SLASH] = Key::KEY_SLASH;
    m_keyMapping[GLFW_KEY_0] = Key::KEY_0;
    m_keyMapping[GLFW_KEY_1] = Key::KEY_1;
    m_keyMapping[GLFW_KEY_2] = Key::KEY_2;
    m_keyMapping[GLFW_KEY_3] = Key::KEY_3;
    m_keyMapping[GLFW_KEY_4] = Key::KEY_4;
    m_keyMapping[GLFW_KEY_5] = Key::KEY_5;
    m_keyMapping[GLFW_KEY_6] = Key::KEY_6;
    m_keyMapping[GLFW_KEY_7] = Key::KEY_7;
    m_keyMapping[GLFW_KEY_8] = Key::KEY_8;
    m_keyMapping[GLFW_KEY_9] = Key::KEY_9;
    m_keyMapping[GLFW_KEY_SEMICOLON] = Key::KEY_SEMICOLON;
    m_keyMapping[GLFW_KEY_EQUAL] = Key::KEY_EQUAL;
    m_keyMapping[GLFW_KEY_A] = Key::KEY_A;
    m_keyMapping[GLFW_KEY_B] = Key::KEY_B;
    m_keyMapping[GLFW_KEY_C] = Key::KEY_C;
    m_keyMapping[GLFW_KEY_D] = Key::KEY_D;
    m_keyMapping[GLFW_KEY_E] = Key::KEY_E;
    m_keyMapping[GLFW_KEY_F] = Key::KEY_F;
    m_keyMapping[GLFW_KEY_G] = Key::KEY_G;
    m_keyMapping[GLFW_KEY_H] = Key::KEY_H;
    m_keyMapping[GLFW_KEY_I] = Key::KEY_I;
    m_keyMapping[GLFW_KEY_J] = Key::KEY_J;
    m_keyMapping[GLFW_KEY_K] = Key::KEY_K;
    m_keyMapping[GLFW_KEY_L] = Key::KEY_L;
    m_keyMapping[GLFW_KEY_M] = Key::KEY_M;
    m_keyMapping[GLFW_KEY_N] = Key::KEY_N;
    m_keyMapping[GLFW_KEY_O] = Key::KEY_O;
    m_keyMapping[GLFW_KEY_P] = Key::KEY_P;
    m_keyMapping[GLFW_KEY_Q] = Key::KEY_Q;
    m_keyMapping[GLFW_KEY_R] = Key::KEY_R;
    m_keyMapping[GLFW_KEY_S] = Key::KEY_S;
    m_keyMapping[GLFW_KEY_T] = Key::KEY_T;
    m_keyMapping[GLFW_KEY_U] = Key::KEY_U;
    m_keyMapping[GLFW_KEY_V] = Key::KEY_V;
    m_keyMapping[GLFW_KEY_W] = Key::KEY_W;
    m_keyMapping[GLFW_KEY_X] = Key::KEY_X;
    m_keyMapping[GLFW_KEY_Y] = Key::KEY_Y;
    m_keyMapping[GLFW_KEY_Z] = Key::KEY_Z;

    m_keyMapping[GLFW_KEY_LEFT_BRACKET] = Key::KEY_LEFT_BRACKET;
    m_keyMapping[GLFW_KEY_BACKSLASH] = Key::KEY_BACKSLASH;
    m_keyMapping[GLFW_KEY_RIGHT_BRACKET] = Key::KEY_RIGHT_BRACKET;
    m_keyMapping[GLFW_KEY_GRAVE_ACCENT] = Key::KEY_GRAVE_ACCENT;
    m_keyMapping[GLFW_KEY_WORLD_1] = Key::KEY_WORLD_1;
    m_keyMapping[GLFW_KEY_WORLD_2] = Key::KEY_WORLD_2;
    m_keyMapping[GLFW_KEY_ESCAPE] = Key::KEY_ESCAPE;
    m_keyMapping[GLFW_KEY_ENTER] = Key::KEY_ENTER;
    m_keyMapping[GLFW_KEY_TAB] = Key::KEY_TAB;
    m_keyMapping[GLFW_KEY_BACKSPACE] = Key::KEY_BACKSPACE;
    m_keyMapping[GLFW_KEY_INSERT] = Key::KEY_INSERT;
    m_keyMapping[GLFW_KEY_DELETE] = Key::KEY_DELETE;
    m_keyMapping[GLFW_KEY_RIGHT] = Key::KEY_RIGHT;
    m_keyMapping[GLFW_KEY_LEFT] = Key::KEY_LEFT;
    m_keyMapping[GLFW_KEY_DOWN] = Key::KEY_DOWN;
    m_keyMapping[GLFW_KEY_UP] = Key::KEY_UP;
    m_keyMapping[GLFW_KEY_PAGE_UP] = Key::KEY_PAGE_UP;
    m_keyMapping[GLFW_KEY_PAGE_DOWN] = Key::KEY_PAGE_DOWN;
    m_keyMapping[GLFW_KEY_HOME] = Key::KEY_HOME;
    m_keyMapping[GLFW_KEY_END] = Key::KEY_END;
    m_keyMapping[GLFW_KEY_CAPS_LOCK] = Key::KEY_CAPS_LOCK;
    m_keyMapping[GLFW_KEY_SCROLL_LOCK] = Key::KEY_SCROLL_LOCK;
    m_keyMapping[GLFW_KEY_NUM_LOCK] = Key::KEY_NUM_LOCK;
    m_keyMapping[GLFW_KEY_PRINT_SCREEN] = Key::KEY_PRINT_SCREEN;
    m_keyMapping[GLFW_KEY_PAUSE] = Key::KEY_PAUSE;
    m_keyMapping[GLFW_KEY_F1] = Key::KEY_F1;
    m_keyMapping[GLFW_KEY_F2] = Key::KEY_F2;
    m_keyMapping[GLFW_KEY_F3] = Key::KEY_F3;
    m_keyMapping[GLFW_KEY_F4] = Key::KEY_F4;
    m_keyMapping[GLFW_KEY_F5] = Key::KEY_F5;
    m_keyMapping[GLFW_KEY_F6] = Key::KEY_F6;
    m_keyMapping[GLFW_KEY_F7] = Key::KEY_F7;
    m_keyMapping[GLFW_KEY_F8] = Key::KEY_F8;
    m_keyMapping[GLFW_KEY_F9] = Key::KEY_F9;
    m_keyMapping[GLFW_KEY_F10] = Key::KEY_F10;
    m_keyMapping[GLFW_KEY_F11] = Key::KEY_F11;
    m_keyMapping[GLFW_KEY_F12] = Key::KEY_F12;
    m_keyMapping[GLFW_KEY_F13] = Key::KEY_F13;
    m_keyMapping[GLFW_KEY_F14] = Key::KEY_F14;
    m_keyMapping[GLFW_KEY_F15] = Key::KEY_F15;
    m_keyMapping[GLFW_KEY_F16] = Key::KEY_F16;
    m_keyMapping[GLFW_KEY_F17] = Key::KEY_F17;
    m_keyMapping[GLFW_KEY_F18] = Key::KEY_F18;
    m_keyMapping[GLFW_KEY_F19] = Key::KEY_F19;
    m_keyMapping[GLFW_KEY_F20] = Key::KEY_F20;
    m_keyMapping[GLFW_KEY_F21] = Key::KEY_F21;
    m_keyMapping[GLFW_KEY_F22] = Key::KEY_F22;
    m_keyMapping[GLFW_KEY_F23] = Key::KEY_F23;
    m_keyMapping[GLFW_KEY_F24] = Key::KEY_F24;
    m_keyMapping[GLFW_KEY_F25] = Key::KEY_F25;
    m_keyMapping[GLFW_KEY_KP_0] = Key::KEY_KP_0;
    m_keyMapping[GLFW_KEY_KP_1] = Key::KEY_KP_1;
    m_keyMapping[GLFW_KEY_KP_2] = Key::KEY_KP_2;
    m_keyMapping[GLFW_KEY_KP_3] = Key::KEY_KP_3;
    m_keyMapping[GLFW_KEY_KP_4] = Key::KEY_KP_4;
    m_keyMapping[GLFW_KEY_KP_5] = Key::KEY_KP_5;
    m_keyMapping[GLFW_KEY_KP_6] = Key::KEY_KP_6;
    m_keyMapping[GLFW_KEY_KP_7] = Key::KEY_KP_7;
    m_keyMapping[GLFW_KEY_KP_8] = Key::KEY_KP_8;
    m_keyMapping[GLFW_KEY_KP_9] = Key::KEY_KP_9;
    m_keyMapping[GLFW_KEY_KP_DECIMAL] = Key::KEY_KP_DECIMAL;
    m_keyMapping[GLFW_KEY_KP_DIVIDE] = Key::KEY_KP_DIVIDE;
    m_keyMapping[GLFW_KEY_KP_MULTIPLY] = Key::KEY_KP_MULTIPLY;
    m_keyMapping[GLFW_KEY_KP_SUBTRACT] = Key::KEY_KP_SUBTRACT;
    m_keyMapping[GLFW_KEY_KP_ADD] = Key::KEY_KP_ADD;
    m_keyMapping[GLFW_KEY_KP_ENTER] = Key::KEY_KP_ENTER;
    m_keyMapping[GLFW_KEY_KP_EQUAL] = Key::KEY_KP_EQUAL;
    m_keyMapping[GLFW_KEY_LEFT_SHIFT] = Key::LeftShift;
    m_keyMapping[GLFW_KEY_LEFT_CONTROL] = Key::LeftCtrl;
    m_keyMapping[GLFW_KEY_LEFT_ALT] = Key::LeftAlt;
    m_keyMapping[GLFW_KEY_LEFT_SUPER] = Key::KEY_LEFT_SUPER;
    m_keyMapping[GLFW_KEY_RIGHT_SHIFT] = Key::RightShift;
    m_keyMapping[GLFW_KEY_RIGHT_CONTROL] = Key::RightCtrl;
    m_keyMapping[GLFW_KEY_RIGHT_ALT] = Key::RightAlt;
    m_keyMapping[GLFW_KEY_RIGHT_SUPER] = Key::KEY_RIGHT_SUPER;
    m_keyMapping[GLFW_KEY_MENU] = Key::KEY_MENU;
}

}  // namespace cave

// @TODO: get rid of this, discusting af
WARNING_DISABLE(4127, "-Wunused-parameter")
WARNING_DISABLE(4189, "-Wunused-parameter")
#include <imgui/backends/imgui_impl_glfw.cpp>
