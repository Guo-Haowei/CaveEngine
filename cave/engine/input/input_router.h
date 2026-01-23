#pragma once
#include "engine/input/action_consumer_interface.h"

namespace cave {

class InputRouter {
public:
    void Register(IActionConsumer* p_comsumer);
    void Unregister(IActionConsumer* p_comsumer);

    void Dispatch(const ActionEvent& e);

private:
    void Sort();
    std::vector<IActionConsumer*> m_consumers;
};

}  // namespace cave
