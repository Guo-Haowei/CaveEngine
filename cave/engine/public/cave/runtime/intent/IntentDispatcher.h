// =============================================================================
// File: cave/framework/intent/IntentDispatcher.h
// =============================================================================
#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "cave/core/diagnostics/Command.h"
#include "cave/runtime/intent/IIntentHandler.h"
#include "cave/runtime/intent/Intent.h"

namespace cave {

struct CommandArgs;
struct CommandContext;

class IntentDispatcher {
public:
    IntentDispatcher();

    bool addHandler(IntentTypeId type_id, IIntentHandler* handler);
    bool removeHandler(IntentTypeId type_id, IIntentHandler* handler);

    template<IntentType T>
    void addHandler(IIntentHandler* handler) {
        addHandler(T::TypeId, handler);
    }

    template<IntentType T>
    void removeHandler(IIntentHandler* handler) {
        removeHandler(T::TypeId, handler);
    }

    template<IntentType T, typename... Args>
    auto queue(Args&&... args) -> T& {
        auto intent = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *intent;

        pending_.emplace_back(std::move(intent));
        return ref;
    }

    void flush();

#if USING(USE_COMMAND)
    bool Cmd_dump(CommandContext& ctx, const CommandArgs& args);
#endif

private:
    void dispatchOne(Intent& intent);

    std::unordered_map<IntentTypeId, std::vector<IIntentHandler*>> handlers_;
    std::vector<std::unique_ptr<Intent>> pending_;
};

}  // namespace cave
