#include "glfw_keyboard_mouse_device.h"

#include <glfw/glfw3.h>

namespace cave {

// codes for mouse buttons
static constexpr uint32_t kMouseButtonBase = 1000;

GlfwKeyboardMouseDevice::GlfwKeyboardMouseDevice(InputDeviceId p_id, GLFWwindow* p_window)
    : m_id(p_id)
    , m_window(p_window) {
}

void GlfwKeyboardMouseDevice::Poll(std::vector<Event>& p_out_events) {
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

uint64_t GlfwKeyboardMouseDevice::NowUs() {
    using namespace std::chrono;
    const auto now = time_point_cast<microseconds>(steady_clock::now());
    return static_cast<uint64_t>(now.time_since_epoch().count());
}

void GlfwKeyboardMouseDevice::Push(Event p_event) {
    m_queue.push_back(p_event);
}

uint32_t GlfwKeyboardMouseDevice::MapGlfwMouseButtonToCode(int p_glfw_button) {
    // Keep mouse buttons separate from keyboard codes.
    return kMouseButtonBase + static_cast<uint32_t>(p_glfw_button);
}

uint32_t GlfwKeyboardMouseDevice::MapGlfwKeyToCode(int p_glfw_key) {
    unused(p_glfw_key);
    return 0;
}

void GlfwKeyboardMouseDevice::KeyCallback(GLFWwindow* p_window, int p_key, int, int p_action, int) {
    unused(p_key);
    unused(p_action);

    if (GlfwKeyboardMouseDevice* self = Get(p_window)) {
    }

#if 0
    const uint32_t code = MapGlfwKeyToCode(key);
    if (code == ToCode(Key::Unknown)) return;

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        InputEvent e{};
        e.type = InputEventType::ButtonDown;
        e.device = self->m_id;
        e.code = code;
        e.timestamp_us = NowUs();
        self->Push(e);
    } else if (action == GLFW_RELEASE) {
        InputEvent e{};
        e.type = InputEventType::ButtonUp;
        e.device = self->m_id;
        e.code = code;
        e.timestamp_us = NowUs();
        self->Push(e);
    }
#endif
}

void GlfwKeyboardMouseDevice::CharCallback(GLFWwindow* p_window, unsigned int) {
    if (GlfwKeyboardMouseDevice* self = Get(p_window)) {
    }
#if 0
    auto* self = Get(w);
    if (!self) return;

    InputEvent e{};
    e.type = InputEventType::TextInput;
    e.device = self->m_id;
    e.code = static_cast<uint32_t>(codepoint);
    e.timestamp_us = NowUs();
    self->Push(e);
#endif
}

void GlfwKeyboardMouseDevice::MouseButtonCallback(GLFWwindow* p_window, int p_button, int p_action, int /*mods*/) {
    unused(p_button);
    unused(p_action);
    if (GlfwKeyboardMouseDevice* self = Get(p_window)) {
    }

#if 0
    InputEvent e{};
    e.device = self->m_id;
    e.code = MapGlfwMouseButtonToCode(button);
    e.timestamp_us = NowUs();

    if (action == GLFW_PRESS)
        e.type = InputEventType::ButtonDown;
    else if (action == GLFW_RELEASE)
        e.type = InputEventType::ButtonUp;
    else
        return;

    self->Push(e);
#endif
}

void GlfwKeyboardMouseDevice::CursorPosCallback(GLFWwindow* p_window, double p_x, double p_y) {
    if (GlfwKeyboardMouseDevice* self = Get(p_window)) {
        Vector2f new_pos(p_x, p_y);

        auto e = std::make_shared<InputEventMouseMove>(
            self->m_buttons,
            self->m_prev_buttons,
            new_pos,
            self->m_pointer_pos);
#if 0
    InputEvent e{};
    e.type = InputEventType::MouseMove;
    e.device = self->m_id;
    e.v0 = static_cast<float>(x);
    e.v1 = static_cast<float>(y);
    e.timestamp_us = NowUs();
    self->Push(e);
#endif
        self->Push(e);
    }
}

void GlfwKeyboardMouseDevice::ScrollCallback(GLFWwindow* p_window, double, double p_y_offset) {
    if (GlfwKeyboardMouseDevice* self = Get(p_window)) {
        auto e = std::make_shared<InputEventMouseWheel>(
            self->m_buttons,
            self->m_prev_buttons,
            self->m_pointer_pos,
            static_cast<float>(p_y_offset));
#if 0
    InputEvent e{};
    e.type = InputEventType::MouseWheel;
    e.device = self->m_id;
    e.v0 = static_cast<float>(yoff);
    e.timestamp_us = NowUs();
    self->Push(e);
#endif
        self->Push(e);
    }
}

}  // namespace cave
