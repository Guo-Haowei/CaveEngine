#pragma once
#include "engine/private/input/input_device_interface.h"

struct GLFWwindow;

namespace cave {

enum class Key : uint16_t;

class GlfwKeyboardMouseDevice : public IInputDevice {
public:
    GlfwKeyboardMouseDevice(InputDeviceId p_dev_id);

    InputDeviceType Type() const override { return InputDeviceType::KeyboardMouse; }
    InputDeviceId Id() const override { return m_dev_id; }

    void Poll(std::vector<InputEvent>& p_out_events) override;

    void InstallCallbacks(GLFWwindow* p_window);

private:
    Key MapGlfwKeyToCode(int p_glfw_key);
    Key MapGlfwMouseButtonToCode(int p_glfw_button);

    static GlfwKeyboardMouseDevice* Get(GLFWwindow* p_window);

    static void KeyCallback(GLFWwindow* p_window, int p_key, int p_scancode, int p_action, int p_mods);
    static void CharCallback(GLFWwindow* p_window, unsigned int p_codepoint);
    static void MouseButtonCallback(GLFWwindow* p_window, int p_button, int p_action, int p_mods);
    static void CursorPosCallback(GLFWwindow* p_window, double p_x, double p_y);
    static void ScrollCallback(GLFWwindow* p_window, double p_x_offset, double p_y_offset);

    void Push(InputEvent e);

    InputDeviceId m_dev_id{};

    std::vector<GLFWwindow*> m_windows;
    std::deque<InputEvent> m_queue;
    std::unordered_map<int, Key> m_key_mapping;
};

}  // namespace cave
