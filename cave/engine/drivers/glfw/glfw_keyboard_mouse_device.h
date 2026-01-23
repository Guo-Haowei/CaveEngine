#pragma once
#include "engine/input/input_device_interface.h"

struct GLFWwindow;

namespace cave {

class GlfwKeyboardMouseDevice : public IInputDevice {
public:
    GlfwKeyboardMouseDevice(InputDeviceId p_id, GLFWwindow* p_window);

    InputDeviceType Type() const override { return InputDeviceType::KeyboardMouse; }
    InputDeviceId Id() const override { return m_id; }

    void Poll(std::vector<Event>& p_out_events) override;

    void InstallCallbacks();

private:
    static GlfwKeyboardMouseDevice* Get(GLFWwindow* p_window);

    static void KeyCallback(GLFWwindow* p_window, int p_key, int p_scancode, int p_action, int p_mods);
    static void CharCallback(GLFWwindow* p_window, unsigned int p_codepoint);
    static void MouseButtonCallback(GLFWwindow* p_window, int p_button, int p_action, int p_mods);
    static void CursorPosCallback(GLFWwindow* p_window, double p_x, double p_y);
    static void ScrollCallback(GLFWwindow* p_window, double p_x_offset, double p_y_offset);

    // Map GLFW key -> your Key enum code (KeyCodes.h)
    static uint32_t MapGlfwKeyToCode(int p_glfw_key);
    static uint32_t MapGlfwMouseButtonToCode(int p_glfw_button);

    void Push(Event e);

    // @TODO: refactor
    static uint64_t NowUs();

    InputDeviceId m_id{};
    GLFWwindow* m_window{ nullptr };

    std::deque<Event> m_queue;

    // @TODO: refactor
    MouseButtonArray m_buttons;
    MouseButtonArray m_prev_buttons;
    Vector2f m_pointer_pos;
};

}  // namespace cave
