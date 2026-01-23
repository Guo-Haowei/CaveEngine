#pragma once
#include "engine/input/action_consumer_interface.h"
#include "engine/input/raw_input_consumer_interface.h"

namespace cave {

class RawInputRouter {
public:
    void Register(IRawInputConsumer* p_consumer);
    void Unregister(IRawInputConsumer* p_consumer);

    void Dispatch(std::vector<InputEvent>& p_events);

private:
    void Sort();

    std::vector<IRawInputConsumer*> m_consumers;
};

class InputRouter {
public:
    void Register(IActionConsumer* p_consumer);
    void Unregister(IActionConsumer* p_consumer);

    void Dispatch(const ActionEvent& p_action);

private:
    void Sort();

    std::vector<IActionConsumer*> m_consumers;
};

}  // namespace cave
