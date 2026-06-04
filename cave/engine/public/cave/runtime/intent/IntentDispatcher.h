// =============================================================================
// File: engine/public/cave/framework/intent/IntentDispatcher.h
// =============================================================================
#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "cave/runtime/framework/IService.h"
#include "cave/runtime/intent/IIntentHandler.h"
#include "cave/runtime/intent/Intent.h"

namespace cave {

struct CommandArgs;
struct CommandContext;

class IntentDispatcher : public IService {
public:
    IntentDispatcher()
        : IService("IntentDispatcher") {}

    bool AddHandler(IntentTypeId p_intent_id, IIntentHandler* p_handler);
    bool RemoveHandler(IntentTypeId p_intent_id, IIntentHandler* p_handler);

    template<IntentType T>
    void AddHandler(IIntentHandler* p_handler) {
        AddHandler(T::TypeId, p_handler);
    }

    template<IntentType T>
    void RemoveHandler(IIntentHandler* p_handler) {
        RemoveHandler(T::TypeId, p_handler);
    }

    template<IntentType T, typename... Args>
    auto Queue(Args&&... args) -> T& {
        auto intent = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *intent;

        m_pending.emplace_back(std::move(intent));
        return ref;
    }

    void Flush();

protected:
    auto InitializeImpl() -> Result<void> final;
    void FinalizeImpl() final;

private:
    void DispatchOne(Intent& p_intent);
    void IntentDispatcherDump_Cmd(CommandContext& p_ctx, const CommandArgs& p_args);

    std::unordered_map<IntentTypeId, std::vector<IIntentHandler*>> m_handlers;
    std::vector<std::unique_ptr<Intent>> m_pending;
};

}  // namespace cave
