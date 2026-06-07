#include "GlfwKeyboardMouseDevice.h"

#include <glfw/glfw3.h>

#include "cave/runtime/input/KeyCode.h"

namespace cave {

GlfwKeyboardMouseDevice::GlfwKeyboardMouseDevice(InputDeviceId dev_id)
    : dev_id_(dev_id) {

    if (DEV_VERIFY(key_mapping_.empty())) {
        key_mapping_[GLFW_KEY_SPACE] = Key::Space;
        key_mapping_[GLFW_KEY_APOSTROPHE] = Key::Apostrophe;
        key_mapping_[GLFW_KEY_COMMA] = Key::Comma;
        key_mapping_[GLFW_KEY_MINUS] = Key::Minus;
        key_mapping_[GLFW_KEY_PERIOD] = Key::Period;
        key_mapping_[GLFW_KEY_SLASH] = Key::Slash;

        for (uint16_t i = GLFW_KEY_0; i <= GLFW_KEY_9; ++i) {
            const uint16_t offset = i - GLFW_KEY_0;
            key_mapping_[i] = static_cast<Key>(std::to_underlying(Key::_0) + offset);
        }

        key_mapping_[GLFW_KEY_SEMICOLON] = Key::Semicolon;
        key_mapping_[GLFW_KEY_EQUAL] = Key::Equal;

        for (uint16_t i = GLFW_KEY_A; i <= GLFW_KEY_Z; ++i) {
            const uint16_t offset = i - GLFW_KEY_A;
            key_mapping_[i] = static_cast<Key>(std::to_underlying(Key::A) + offset);
        }

        key_mapping_[GLFW_KEY_LEFT_BRACKET] = Key::LeftBracket;
        key_mapping_[GLFW_KEY_BACKSLASH] = Key::Backslash;
        key_mapping_[GLFW_KEY_RIGHT_BRACKET] = Key::RightBracket;
        key_mapping_[GLFW_KEY_GRAVE_ACCENT] = Key::GraveAccent;
        key_mapping_[GLFW_KEY_WORLD_1] = Key::World1;
        key_mapping_[GLFW_KEY_WORLD_2] = Key::World2;
        key_mapping_[GLFW_KEY_ESCAPE] = Key::Escape;
        key_mapping_[GLFW_KEY_ENTER] = Key::Enter;
        key_mapping_[GLFW_KEY_TAB] = Key::Tab;
        key_mapping_[GLFW_KEY_BACKSPACE] = Key::Backspace;
        key_mapping_[GLFW_KEY_INSERT] = Key::Insert;
        key_mapping_[GLFW_KEY_DELETE] = Key::Delete;
        key_mapping_[GLFW_KEY_RIGHT] = Key::Right;
        key_mapping_[GLFW_KEY_LEFT] = Key::Left;
        key_mapping_[GLFW_KEY_DOWN] = Key::Down;
        key_mapping_[GLFW_KEY_UP] = Key::Up;
        key_mapping_[GLFW_KEY_PAGE_UP] = Key::PageUp;
        key_mapping_[GLFW_KEY_PAGE_DOWN] = Key::PageDown;
        key_mapping_[GLFW_KEY_HOME] = Key::Home;
        key_mapping_[GLFW_KEY_END] = Key::End;
        key_mapping_[GLFW_KEY_CAPS_LOCK] = Key::CapsLock;
        key_mapping_[GLFW_KEY_SCROLL_LOCK] = Key::ScrollLock;
        key_mapping_[GLFW_KEY_NUM_LOCK] = Key::NumLock;
        key_mapping_[GLFW_KEY_PRINT_SCREEN] = Key::PrintScreen;
        key_mapping_[GLFW_KEY_PAUSE] = Key::Pause;

        for (uint16_t i = GLFW_KEY_F1; i <= GLFW_KEY_F25; ++i) {
            const uint16_t offset = i - GLFW_KEY_F1;
            key_mapping_[i] = static_cast<Key>(std::to_underlying(Key::F1) + offset);
        }

        for (uint16_t i = GLFW_KEY_KP_0; i <= GLFW_KEY_KP_9; ++i) {
            const uint16_t offset = i - GLFW_KEY_KP_0;
            key_mapping_[i] = static_cast<Key>(std::to_underlying(Key::Keypad0) + offset);
        }

        key_mapping_[GLFW_KEY_KP_DECIMAL] = Key::KeypadDecimal;
        key_mapping_[GLFW_KEY_KP_DIVIDE] = Key::KeypadDivide;
        key_mapping_[GLFW_KEY_KP_MULTIPLY] = Key::KeypadMultiply;
        key_mapping_[GLFW_KEY_KP_SUBTRACT] = Key::KeypadSubtract;
        key_mapping_[GLFW_KEY_KP_ADD] = Key::KeypadAdd;
        key_mapping_[GLFW_KEY_KP_ENTER] = Key::KeypadEnter;
        key_mapping_[GLFW_KEY_KP_EQUAL] = Key::KeypadEqual;
        key_mapping_[GLFW_KEY_LEFT_SHIFT] = Key::LeftShift;
        key_mapping_[GLFW_KEY_LEFT_CONTROL] = Key::LeftCtrl;
        key_mapping_[GLFW_KEY_LEFT_ALT] = Key::LeftAlt;
        key_mapping_[GLFW_KEY_LEFT_SUPER] = Key::LeftSuper;
        key_mapping_[GLFW_KEY_RIGHT_SHIFT] = Key::RightShift;
        key_mapping_[GLFW_KEY_RIGHT_CONTROL] = Key::RightCtrl;
        key_mapping_[GLFW_KEY_RIGHT_ALT] = Key::RightAlt;
        key_mapping_[GLFW_KEY_RIGHT_SUPER] = Key::RightSuper;
        key_mapping_[GLFW_KEY_MENU] = Key::Menu;
    }
}

void GlfwKeyboardMouseDevice::poll(std::vector<InputEvent>& out_events) {
    while (!event_queue_.empty()) {
        out_events.push_back(event_queue_.front());
        event_queue_.pop_front();
    }
}

void GlfwKeyboardMouseDevice::InstallCallbacks(GLFWwindow* window) {
    glfwSetWindowUserPointer(window, this);

    glfwSetKeyCallback(window, &GlfwKeyboardMouseDevice::keyCb);
    glfwSetCharCallback(window, &GlfwKeyboardMouseDevice::charCb);
    glfwSetMouseButtonCallback(window, &GlfwKeyboardMouseDevice::mouseButtonCb);
    glfwSetCursorPosCallback(window, &GlfwKeyboardMouseDevice::cursorPosCb);
    glfwSetScrollCallback(window, &GlfwKeyboardMouseDevice::scrollCb);

    windows_.push_back(window);
}

GlfwKeyboardMouseDevice* GlfwKeyboardMouseDevice::getDevice(GLFWwindow* window) {
    return static_cast<GlfwKeyboardMouseDevice*>(glfwGetWindowUserPointer(window));
}

void GlfwKeyboardMouseDevice::queueEvent(const InputEvent& event) {
    event_queue_.push_back(event);
}

Key GlfwKeyboardMouseDevice::mapGlfwMouseButtonToCode(int button) {
    switch (button) {
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

Key GlfwKeyboardMouseDevice::mapGlfwKeyToCode(int key) {
    if (auto it = key_mapping_.find(key); it != key_mapping_.end()) {
        return it->second;
    }

    return Key::None;
}

void GlfwKeyboardMouseDevice::keyCb(GLFWwindow* window,
                                    int key,
                                    int,
                                    int action, int) {

    if (GlfwKeyboardMouseDevice* self = getDevice(window)) {
        if (Key code = self->mapGlfwKeyToCode(key); code != Key::None) {
            InputEventType type = InputEventType::ButtonDown;
            switch (action) {
                case GLFW_PRESS:
                case GLFW_REPEAT:
                    break;
                case GLFW_RELEASE:
                    type = InputEventType::ButtonUp;
                    break;
                default:
                    return;
            }
            InputEvent e(type, self->dev_id_);

            e.code = std::to_underlying(code);
            self->queueEvent(e);
        }
    }
}

void GlfwKeyboardMouseDevice::charCb(GLFWwindow* window, unsigned int code) {
    if (GlfwKeyboardMouseDevice* self = getDevice(window)) {
        self->queueEvent(InputEvent::textInput(self->dev_id_, code));
    }
}

void GlfwKeyboardMouseDevice::mouseButtonCb(GLFWwindow* window, int button, int action, int /*mods*/) {
    if (GlfwKeyboardMouseDevice* self = getDevice(window)) {
        if (Key code = self->mapGlfwMouseButtonToCode(button); code != Key::None) {
            InputEventType type = InputEventType::ButtonDown;
            switch (action) {
                case GLFW_PRESS:
                    break;
                case GLFW_RELEASE:
                    type = InputEventType::ButtonUp;
                    break;
                default:
                    return;
            }

            InputEvent e(type, self->dev_id_);
            e.code = std::to_underlying(code);
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            e.x = static_cast<float>(x);
            e.y = static_cast<float>(y);

            self->queueEvent(e);
        }
    }
}

void GlfwKeyboardMouseDevice::cursorPosCb(GLFWwindow* window, double x, double y) {
    if (GlfwKeyboardMouseDevice* self = getDevice(window)) {
        self->queueEvent(InputEvent::mouseMove(self->dev_id_,
                                               static_cast<float>(x),
                                               static_cast<float>(y)));
    }
}

void GlfwKeyboardMouseDevice::scrollCb(GLFWwindow* window, double dx, double dy) {
    if (GlfwKeyboardMouseDevice* self = getDevice(window)) {
        self->queueEvent(InputEvent::mouseWheel(self->dev_id_,
                                                static_cast<float>(dx),
                                                static_cast<float>(dy)));
    }
}

}  // namespace cave
