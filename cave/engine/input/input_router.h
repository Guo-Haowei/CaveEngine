#pragma once
#include "engine/input/action_consumer_interface.h"

// @TODO: refactor
#include "engine/input/input_event.h"

namespace cave {

enum class HandleInputResult : uint8_t {
    Handled,
    NotHandled,
};

class IInputHandler {
public:
    virtual HandleInputResult HandleInput(std::shared_ptr<OldInputEvent> p_input_event) = 0;
};

class InputRouter {
public:
    void Register(IActionConsumer* p_comsumer);
    void Unregister(IActionConsumer* p_comsumer);

    void Dispatch(const ActionEvent& e);

    //[[deprecated]]
    void Route(std::shared_ptr<OldInputEvent> p_input_event);

    //[[deprecated]]
    void PushHandler(IInputHandler* p_handler);

    //[[deprecated]]
    IInputHandler* PopHandler();

private:
    void Sort();
    std::vector<IActionConsumer*> m_consumers;
};

}  // namespace cave
