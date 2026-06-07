#pragma once
#include "cave/runtime/input/IInputDevice.h"

struct GLFWwindow;

namespace cave {

enum class Key : uint16_t;

class GlfwKeyboardMouseDevice : public IInputDevice {
public:
    GlfwKeyboardMouseDevice(InputDeviceId dev_id);

    InputDeviceType type() const override { return InputDeviceType::KeyboardMouse; }
    InputDeviceId id() const override { return dev_id_; }

    void poll(std::vector<InputEvent>& out_events) override;

    void InstallCallbacks(GLFWwindow* window);

private:
    Key mapGlfwKeyToCode(int key);
    Key mapGlfwMouseButtonToCode(int button);

    static GlfwKeyboardMouseDevice* getDevice(GLFWwindow* window);

    static void keyCb(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void charCb(GLFWwindow* window, unsigned int codepoint);
    static void mouseButtonCb(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCb(GLFWwindow* window, double x, double y);
    static void scrollCb(GLFWwindow* window, double dx, double dy);

    void queueEvent(const InputEvent& e);

    InputDeviceId dev_id_{};

    std::vector<GLFWwindow*> windows_;
    std::deque<InputEvent> event_queue_;
    std::unordered_map<int, Key> key_mapping_;
};

}  // namespace cave
