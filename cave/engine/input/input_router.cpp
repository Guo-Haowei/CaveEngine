#include "input_router.h"

namespace cave {

void InputRouter::Register(IActionConsumer* p_consumer) {
    if (!p_consumer) return;
    for (auto* it : m_consumers) {
        if (it == p_consumer) return;
    }
    m_consumers.push_back(p_consumer);
    Sort();
}

void InputRouter::Unregister(IActionConsumer* p_consumer) {
    m_consumers.erase(
        std::remove(m_consumers.begin(), m_consumers.end(), p_consumer),
        m_consumers.end());
}

void InputRouter::Sort() {
    std::sort(m_consumers.begin(), m_consumers.end(),
              [](const IActionConsumer* a, const IActionConsumer* b) {
                  return a->GetPriority() > b->GetPriority();
              });
}

void InputRouter::Dispatch(const ActionEvent& p_consumer) {
    for (auto* c : m_consumers) {
        if (c && c->OnAction(p_consumer)) {
            return;
        }
    }
}

void InputRouter::Route(std::shared_ptr<InputEvent>) {
}

void InputRouter::PushHandler(IInputHandler*) {
}

IInputHandler* InputRouter::PopHandler() {
    return nullptr;
}

}  // namespace cave
