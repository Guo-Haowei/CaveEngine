#include "GlfwKeyboardMouseDevice.h"

#include <glfw/glfw3.h>

#include "cave/runtime/input/KeyCode.h"

namespace cave {

GlfwKeyboardMouseDevice::GlfwKeyboardMouseDevice(InputDeviceId p_dev_id)
    : m_dev_id(p_dev_id) {

    if (DEV_VERIFY(m_key_mapping.empty())) {
        m_key_mapping[GLFW_KEY_SPACE] = Key::Space;
        m_key_mapping[GLFW_KEY_APOSTROPHE] = Key::Apostrophe;
        m_key_mapping[GLFW_KEY_COMMA] = Key::Comma;
        m_key_mapping[GLFW_KEY_MINUS] = Key::Minus;
        m_key_mapping[GLFW_KEY_PERIOD] = Key::Period;
        m_key_mapping[GLFW_KEY_SLASH] = Key::Slash;

        for (uint16_t i = GLFW_KEY_0; i <= GLFW_KEY_9; ++i) {
            const uint16_t offset = i - GLFW_KEY_0;
            m_key_mapping[i] = static_cast<Key>(std::to_underlying(Key::_0) + offset);
        }

        m_key_mapping[GLFW_KEY_SEMICOLON] = Key::Semicolon;
        m_key_mapping[GLFW_KEY_EQUAL] = Key::Equal;

        for (uint16_t i = GLFW_KEY_A; i <= GLFW_KEY_Z; ++i) {
            const uint16_t offset = i - GLFW_KEY_A;
            m_key_mapping[i] = static_cast<Key>(std::to_underlying(Key::A) + offset);
        }

        m_key_mapping[GLFW_KEY_LEFT_BRACKET] = Key::LeftBracket;
        m_key_mapping[GLFW_KEY_BACKSLASH] = Key::Backslash;
        m_key_mapping[GLFW_KEY_RIGHT_BRACKET] = Key::RightBracket;
        m_key_mapping[GLFW_KEY_GRAVE_ACCENT] = Key::GraveAccent;
        m_key_mapping[GLFW_KEY_WORLD_1] = Key::World1;
        m_key_mapping[GLFW_KEY_WORLD_2] = Key::World2;
        m_key_mapping[GLFW_KEY_ESCAPE] = Key::Escape;
        m_key_mapping[GLFW_KEY_ENTER] = Key::Enter;
        m_key_mapping[GLFW_KEY_TAB] = Key::Tab;
        m_key_mapping[GLFW_KEY_BACKSPACE] = Key::Backspace;
        m_key_mapping[GLFW_KEY_INSERT] = Key::Insert;
        m_key_mapping[GLFW_KEY_DELETE] = Key::Delete;
        m_key_mapping[GLFW_KEY_RIGHT] = Key::Right;
        m_key_mapping[GLFW_KEY_LEFT] = Key::Left;
        m_key_mapping[GLFW_KEY_DOWN] = Key::Down;
        m_key_mapping[GLFW_KEY_UP] = Key::Up;
        m_key_mapping[GLFW_KEY_PAGE_UP] = Key::PageUp;
        m_key_mapping[GLFW_KEY_PAGE_DOWN] = Key::PageDown;
        m_key_mapping[GLFW_KEY_HOME] = Key::Home;
        m_key_mapping[GLFW_KEY_END] = Key::End;
        m_key_mapping[GLFW_KEY_CAPS_LOCK] = Key::CapsLock;
        m_key_mapping[GLFW_KEY_SCROLL_LOCK] = Key::ScrollLock;
        m_key_mapping[GLFW_KEY_NUM_LOCK] = Key::NumLock;
        m_key_mapping[GLFW_KEY_PRINT_SCREEN] = Key::PrintScreen;
        m_key_mapping[GLFW_KEY_PAUSE] = Key::Pause;

        for (uint16_t i = GLFW_KEY_F1; i <= GLFW_KEY_F25; ++i) {
            const uint16_t offset = i - GLFW_KEY_F1;
            m_key_mapping[i] = static_cast<Key>(std::to_underlying(Key::F1) + offset);
        }

        for (uint16_t i = GLFW_KEY_KP_0; i <= GLFW_KEY_KP_9; ++i) {
            const uint16_t offset = i - GLFW_KEY_KP_0;
            m_key_mapping[i] = static_cast<Key>(std::to_underlying(Key::Keypad0) + offset);
        }

        m_key_mapping[GLFW_KEY_KP_DECIMAL] = Key::KeypadDecimal;
        m_key_mapping[GLFW_KEY_KP_DIVIDE] = Key::KeypadDivide;
        m_key_mapping[GLFW_KEY_KP_MULTIPLY] = Key::KeypadMultiply;
        m_key_mapping[GLFW_KEY_KP_SUBTRACT] = Key::KeypadSubtract;
        m_key_mapping[GLFW_KEY_KP_ADD] = Key::KeypadAdd;
        m_key_mapping[GLFW_KEY_KP_ENTER] = Key::KeypadEnter;
        m_key_mapping[GLFW_KEY_KP_EQUAL] = Key::KeypadEqual;
        m_key_mapping[GLFW_KEY_LEFT_SHIFT] = Key::LeftShift;
        m_key_mapping[GLFW_KEY_LEFT_CONTROL] = Key::LeftCtrl;
        m_key_mapping[GLFW_KEY_LEFT_ALT] = Key::LeftAlt;
        m_key_mapping[GLFW_KEY_LEFT_SUPER] = Key::LeftSuper;
        m_key_mapping[GLFW_KEY_RIGHT_SHIFT] = Key::RightShift;
        m_key_mapping[GLFW_KEY_RIGHT_CONTROL] = Key::RightCtrl;
        m_key_mapping[GLFW_KEY_RIGHT_ALT] = Key::RightAlt;
        m_key_mapping[GLFW_KEY_RIGHT_SUPER] = Key::RightSuper;
        m_key_mapping[GLFW_KEY_MENU] = Key::Menu;
    }
}

void GlfwKeyboardMouseDevice::Poll(std::vector<InputEvent>& p_out_events) {
    while (!m_queue.empty()) {
        p_out_events.push_back(m_queue.front());
        m_queue.pop_front();
    }
}

void GlfwKeyboardMouseDevice::InstallCallbacks(GLFWwindow* p_window) {
    glfwSetWindowUserPointer(p_window, this);

    glfwSetKeyCallback(p_window, &GlfwKeyboardMouseDevice::KeyCallback);
    glfwSetCharCallback(p_window, &GlfwKeyboardMouseDevice::CharCallback);
    glfwSetMouseButtonCallback(p_window, &GlfwKeyboardMouseDevice::MouseButtonCallback);
    glfwSetCursorPosCallback(p_window, &GlfwKeyboardMouseDevice::CursorPosCallback);
    glfwSetScrollCallback(p_window, &GlfwKeyboardMouseDevice::ScrollCallback);

    m_windows.push_back(p_window);
}

GlfwKeyboardMouseDevice* GlfwKeyboardMouseDevice::Get(GLFWwindow* p_window) {
    return static_cast<GlfwKeyboardMouseDevice*>(glfwGetWindowUserPointer(p_window));
}

void GlfwKeyboardMouseDevice::Push(InputEvent p_event) {
    m_queue.push_back(p_event);
}

Key GlfwKeyboardMouseDevice::MapGlfwMouseButtonToCode(int p_glfw_button) {
    switch (p_glfw_button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            return Key::LMB;
        case GLFW_MOUSE_BUTTON_RIGHT:
            return Key::RMB;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            return Key::MMB;
        default:
            return Key::None;
    }
}

Key GlfwKeyboardMouseDevice::MapGlfwKeyToCode(int p_glfw_key) {
    if (auto it = m_key_mapping.find(p_glfw_key); it != m_key_mapping.end()) {
        return it->second;
    }

    return Key::None;
}

void GlfwKeyboardMouseDevice::KeyCallback(GLFWwindow* p_window,
                                          int p_key,
                                          int,
                                          int p_action, int) {

    if (GlfwKeyboardMouseDevice* self = Get(p_window)) {
        if (Key code = self->MapGlfwKeyToCode(p_key); code != Key::None) {
            InputEventType type = InputEventType::ButtonDown;
            switch (p_action) {
                case GLFW_PRESS:
                case GLFW_REPEAT:
                    break;
                case GLFW_RELEASE:
                    type = InputEventType::ButtonUp;
                    break;
                default:
                    return;
            }
            InputEvent e(type, self->m_dev_id);

            e.code = std::to_underlying(code);
            self->Push(e);
        }
    }
}

void GlfwKeyboardMouseDevice::CharCallback(GLFWwindow* p_window, unsigned int p_code) {
    if (GlfwKeyboardMouseDevice* self = Get(p_window)) {
        self->Push(InputEvent::textInput(self->m_dev_id, p_code));
    }
}

void GlfwKeyboardMouseDevice::MouseButtonCallback(GLFWwindow* p_window, int p_button, int p_action, int /*mods*/) {
    if (GlfwKeyboardMouseDevice* self = Get(p_window)) {
        if (Key code = self->MapGlfwMouseButtonToCode(p_button); code != Key::None) {
            InputEventType type = InputEventType::ButtonDown;
            switch (p_action) {
                case GLFW_PRESS:
                    break;
                case GLFW_RELEASE:
                    type = InputEventType::ButtonUp;
                    break;
                default:
                    return;
            }

            InputEvent e(type, self->m_dev_id);
            e.code = std::to_underlying(code);
            double x, y;
            glfwGetCursorPos(p_window, &x, &y);
            e.x = static_cast<float>(x);
            e.y = static_cast<float>(y);

            self->Push(e);
        }
    }
}

void GlfwKeyboardMouseDevice::CursorPosCallback(GLFWwindow* p_window, double p_x, double p_y) {
    if (GlfwKeyboardMouseDevice* self = Get(p_window)) {
        self->Push(InputEvent::mouseMove(self->m_dev_id,
                                         static_cast<float>(p_x),
                                         static_cast<float>(p_y)));
    }
}

void GlfwKeyboardMouseDevice::ScrollCallback(GLFWwindow* p_window, double p_x_offset, double p_y_offset) {
    if (GlfwKeyboardMouseDevice* self = Get(p_window)) {
        self->Push(InputEvent::mouseWheel(self->m_dev_id,
                                          static_cast<float>(p_x_offset),
                                          static_cast<float>(p_y_offset)));
    }
}

}  // namespace cave
