#include "glfw_keyboard_mouse_device.h"

#include <glfw/glfw3.h>

#include "engine/input/input_code.h"

namespace cave {

// codes for mouse buttons
static constexpr uint32_t kMouseButtonBase = 1000;

static uint64_t NowUs() {
    using namespace std::chrono;
    const auto now = time_point_cast<microseconds>(steady_clock::now());
    return static_cast<uint64_t>(now.time_since_epoch().count());
}

GlfwKeyboardMouseDevice::GlfwKeyboardMouseDevice(InputDeviceId p_id, GLFWwindow* p_window)
    : m_id(p_id)
    , m_window(p_window) {
}

void GlfwKeyboardMouseDevice::Poll(std::vector<InputEvent>& p_out_events) {
    while (!m_queue.empty()) {
        p_out_events.push_back(m_queue.front());
        m_queue.pop_front();
    }
}

void GlfwKeyboardMouseDevice::InstallCallbacks() {
    glfwSetWindowUserPointer(m_window, this);

    glfwSetKeyCallback(m_window, &GlfwKeyboardMouseDevice::KeyCallback);
    glfwSetCharCallback(m_window, &GlfwKeyboardMouseDevice::CharCallback);
    glfwSetMouseButtonCallback(m_window, &GlfwKeyboardMouseDevice::MouseButtonCallback);
    glfwSetCursorPosCallback(m_window, &GlfwKeyboardMouseDevice::CursorPosCallback);
    glfwSetScrollCallback(m_window, &GlfwKeyboardMouseDevice::ScrollCallback);
}

GlfwKeyboardMouseDevice* GlfwKeyboardMouseDevice::Get(GLFWwindow* p_window) {
    return static_cast<GlfwKeyboardMouseDevice*>(glfwGetWindowUserPointer(p_window));
}

void GlfwKeyboardMouseDevice::Push(InputEvent p_event) {
    m_queue.push_back(p_event);
}

Key GlfwKeyboardMouseDevice::MapGlfwMouseButtonToCode(int p_glfw_button) {
    DEV_ASSERT(0);
    unused(p_glfw_button);
    return Key::Unknown;
}

Key GlfwKeyboardMouseDevice::MapGlfwKeyToCode(int p_glfw_key) {
    if (auto it = m_keyMapping.find(p_glfw_key); it != m_keyMapping.end()) {
        return it->second;
    }

    return Key::Unknown;
}

void GlfwKeyboardMouseDevice::KeyCallback(GLFWwindow* p_window, int p_key, int, int p_action, int) {

    if (GlfwKeyboardMouseDevice* self = Get(p_window)) {
        if (Key code = self->MapGlfwKeyToCode(p_key); code != Key::Unknown) {
            InputEvent e{};
            switch (p_action) {
                case GLFW_PRESS:
                case GLFW_REPEAT:
                    e.type = InputEventType::ButtonDown;
                    break;
                case GLFW_RELEASE:
                    e.type = InputEventType::ButtonUp;
                    break;
                default:
                    return;
            }

            e.device = self->m_id;
            e.timestamp_us = NowUs();

            e.code = std::to_underlying(code);
            self->Push(e);
        }
    }
}

void GlfwKeyboardMouseDevice::CharCallback(GLFWwindow* p_window, unsigned int p_code) {
    if (GlfwKeyboardMouseDevice* self = Get(p_window)) {
        self->Push(InputEvent::TextInput(self->m_id, NowUs(), p_code));
    }
}

void GlfwKeyboardMouseDevice::MouseButtonCallback(GLFWwindow* p_window, int p_button, int p_action, int /*mods*/) {
    if (GlfwKeyboardMouseDevice* self = Get(p_window)) {
        if (Key code = self->MapGlfwMouseButtonToCode(p_button); code != Key::Unknown) {
            InputEvent e{};
            if (p_action == GLFW_PRESS) {
                e.type = InputEventType::ButtonDown;
            } else if (p_action == GLFW_RELEASE) {
                e.type = InputEventType::ButtonUp;
            } else {
                return;
            }

            e.device = self->m_id;
            e.code = std::to_underlying(code);
            e.timestamp_us = NowUs();

            self->Push(e);
        }
    }
}

void GlfwKeyboardMouseDevice::CursorPosCallback(GLFWwindow* p_window, double p_x, double p_y) {
    if (GlfwKeyboardMouseDevice* self = Get(p_window)) {
        self->Push(InputEvent::MouseMove(self->m_id,
                                         NowUs(),
                                         static_cast<float>(p_x),
                                         static_cast<float>(p_y)));
    }
}

void GlfwKeyboardMouseDevice::ScrollCallback(GLFWwindow* p_window, double, double p_y_offset) {
    if (GlfwKeyboardMouseDevice* self = Get(p_window)) {
        InputEvent e{};
        self->Push(InputEvent::MouseWheel(self->m_id,
                                          NowUs(),
                                          static_cast<float>(p_y_offset)));
    }
}

}  // namespace cave
