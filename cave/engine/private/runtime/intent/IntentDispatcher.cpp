#include "cave/runtime/intent/IntentDispatcher.h"

namespace cave {

void IntentDispatcher::AddHandler(IntentTypeId p_intent_id, IIntentHandler* p_handler) {
    DEV_ASSERT(p_handler);

    auto [it, inserted] = m_handlers.try_emplace(p_intent_id);
    if (!inserted) {
        [[maybe_unused]] const bool no_collision = it->first == p_intent_id;
    }

    it->second.push_back(p_handler);
}

void IntentDispatcher::RemoveHandler(IntentTypeId p_intent_id, IIntentHandler* p_handler) {
    auto it = m_handlers.find(p_intent_id);
    if (it == m_handlers.end()) return;
    std::vector<IIntentHandler*>& handlers = it->second;
    auto it2 = std::remove(handlers.begin(), handlers.end(), p_handler);
    handlers.erase(it2, handlers.end());
}

void IntentDispatcher::Flush() {
    for (const auto& intent : m_intents) {
        DispatchOne(*intent);
    }
    m_intents.clear();
}

void IntentDispatcher::DispatchOne(const Intent& p_intent) {
    auto it = m_handlers.find(p_intent.GetTypeId());
    if (it == m_handlers.end()) {
        LOG_WARN("IntentDispatcher::DispatchOne: no handlers found for intent '{}'", p_intent.GetDebugName());
        return;
    }

    for (IIntentHandler* handler : it->second) {
        if (DEV_VERIFY(handler))
            handler->HandleIntent(p_intent);
    }
}

}  // namespace cave
