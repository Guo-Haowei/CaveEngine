
#pragma once
#include "engine/input/input_event.h"

namespace cave {

enum class HandleInputResult : uint8_t {
    Handled,
    NotHandled,
};

class IInputHandler {
public:
    virtual HandleInputResult HandleInput(std::shared_ptr<InputEvent> p_input_event) = 0;
};

class InputRouter {
public:
    void Route(std::shared_ptr<InputEvent> p_input_event);

    void PushHandler(IInputHandler* p_handler);

    IInputHandler* PopHandler();

private:
    std::vector<IInputHandler*> m_stack;
};

}  // namespace cave
