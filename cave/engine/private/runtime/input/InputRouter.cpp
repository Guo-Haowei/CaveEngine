// =============================================================================
// File: engine/private/runtime/input/InputRouter.cpp
// =============================================================================
#include "InputRouter.h"

namespace cave {

void InputRouter::Register(IInputConsumer* p_consumer) {
    DEV_ASSERT(p_consumer);
    if (!p_consumer) return;

    auto it = std::ranges::find(m_consumers, p_consumer);
    if (it != m_consumers.end()) return;

    m_consumers.push_back(p_consumer);
    Sort();

#if USING(USE_LOG)
    DebugId id = p_consumer->GetDebugId();
    LOG_VERBOSE("InputRouter::Register: register input consumer '{}(id:{})'", id.type, id.uid);
#endif
}

void InputRouter::Unregister(IInputConsumer* p_consumer) {
    m_consumers.erase(
        std::remove(m_consumers.begin(), m_consumers.end(), p_consumer),
        m_consumers.end());

#if USING(USE_LOG)
    DebugId id = p_consumer->GetDebugId();
    LOG_VERBOSE("InputRouter::Unegister: unregister input consumer '{}(id:{})'", id.type, id.uid);
#endif
}

void InputRouter::Dispatch(const InputFrame& p_input) {
    for (auto* c : m_consumers) {
        if (DEV_VERIFY(c)) {
            c->OnEvents(p_input);
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
