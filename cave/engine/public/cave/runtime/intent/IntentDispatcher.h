// =============================================================================
// File: engine/public/cave/framework/intent/IntentDispatcher.h
// =============================================================================
#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "IIntentHandler.h"
#include "Intent.h"

namespace cave {

class IntentDispatcher {
public:
    void AddHandler(IntentTypeId p_intent_id, IIntentHandler* p_handler);
    void RemoveHandler(IntentTypeId p_intent_id, IIntentHandler* p_handler);

    template<typename T, typename... Args>
    auto PushIntent(Args&&... args) -> T& {
        auto intent = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *intent;

        m_intents.emplace_back(std::move(intent));
        return ref;
    }

    void Flush();

private:
    void DispatchOne(const Intent& p_intent);

private:
    std::unordered_map<IntentTypeId, std::vector<IIntentHandler*>> m_handlers;

    std::vector<std::unique_ptr<Intent>> m_intents;
};

}  // namespace cave
