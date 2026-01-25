#include "input_router.h"

namespace cave {

void RawInputRouter::Register(IRawInputConsumer* p_consumer) {
    if (DEV_VERIFY(p_consumer)) {
        for (auto* it : m_consumers) {
            if (it == p_consumer) {
                return;
            }
        }

        m_consumers.push_back(p_consumer);
        Sort();
    }
    LOG_VERBOSE("RawInputRouter::Register: register raw input consumer {}", (void*)p_consumer);
}

void RawInputRouter::Unregister(IRawInputConsumer* p_consumer) {
    m_consumers.erase(
        std::remove(m_consumers.begin(), m_consumers.end(), p_consumer),
        m_consumers.end());
    LOG_VERBOSE("RawInputRouter::Register: unregister raw input consumer {}", (void*)p_consumer);
}

void RawInputRouter::Dispatch(const std::vector<InputEvent>& p_events) {
    for (auto* c : m_consumers) {
        if (DEV_VERIFY(c)) {
            c->OnEvents(p_events);
        }
    }
}

void RawInputRouter::Sort() {
    std::sort(m_consumers.begin(), m_consumers.end(),
              [](const IRawInputConsumer* a, const IRawInputConsumer* b) {
                  return a->GetPriority() > b->GetPriority();
              });
}

void InputRouter::Register(IActionConsumer* p_consumer) {
    if (DEV_VERIFY(p_consumer)) {
        for (auto* it : m_consumers) {
            if (it == p_consumer) {
                return;
            }
        }

        m_consumers.push_back(p_consumer);
        Sort();
    }
    LOG_VERBOSE("InputRouter::Register: register action consumer {}", (void*)p_consumer);
}

void InputRouter::Unregister(IActionConsumer* p_consumer) {
    m_consumers.erase(
        std::remove(m_consumers.begin(), m_consumers.end(), p_consumer),
        m_consumers.end());
    LOG_VERBOSE("InputRouter::Register: unregister action consumer {}", (void*)p_consumer);
}

void InputRouter::Sort() {
    std::sort(m_consumers.begin(), m_consumers.end(),
              [](const IActionConsumer* a, const IActionConsumer* b) {
                  return a->GetPriority() > b->GetPriority();
              });
}

void InputRouter::Dispatch(const ActionEvent& p_action) {
    for (auto* c : m_consumers) {
        if (c && c->OnAction(p_action)) {
            return;
        }
    }
}

}  // namespace cave
