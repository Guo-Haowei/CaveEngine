// =============================================================================
// File: engine/private/runtime/input/InputRouter.cpp
// =============================================================================
#include "InputRouter.h"

namespace cave {

void InputRouter::Register(IInputConsumer* p_consumer) {
    if (DEV_VERIFY(p_consumer)) {
        for (auto* it : m_consumers) {
            if (it == p_consumer) {
                return;
            }
        }

        m_consumers.push_back(p_consumer);
        Sort();
    }
    LOG_VERBOSE("InputRouter::Register: register raw input consumer {}", (void*)p_consumer);
}

void InputRouter::Unregister(IInputConsumer* p_consumer) {
    m_consumers.erase(
        std::remove(m_consumers.begin(), m_consumers.end(), p_consumer),
        m_consumers.end());
    LOG_VERBOSE("InputRouter::Register: unregister raw input consumer {}", (void*)p_consumer);
}

void InputRouter::Dispatch(const std::vector<InputEvent>& p_events) {
    for (auto* c : m_consumers) {
        if (DEV_VERIFY(c)) {
            c->OnEvents(p_events);
        }
    }
}

void InputRouter::Sort() {
    std::sort(m_consumers.begin(), m_consumers.end(),
              [](const IInputConsumer* a, const IInputConsumer* b) {
                  return a->GetPriority() > b->GetPriority();
              });
}

}  // namespace cave
