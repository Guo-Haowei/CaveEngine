// =============================================================================
// File: cave/framework/intent/IntentBus.h
// =============================================================================
#pragma once
#include "cave/core/containers/Containers.h"
#include "cave/core/memory/Pointer.h"
#include "cave/core/diagnostics/Command.h"
#include "cave/runtime/intent/IIntentHandler.h"
#include "cave/runtime/intent/Intent.h"

namespace cave {

struct CommandArgs;
struct CommandContext;

class IntentBus {
public:
    IntentBus();

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
        auto intent = MakeOwner<T>(std::forward<Args>(args)...);
        T& ref = *intent;

        m_pending.emplace_back(std::move(intent));
        return ref;
    }

    void flush();

#if USING(USE_COMMAND)
    bool Cmd_dump(CommandContext& ctx, const CommandArgs& args);
#endif

private:
    void dispatchOne(Intent& intent);

    HashMap<IntentTypeId, Vector<IIntentHandler*>> m_handlers;
    Vector<Owner<Intent>> m_pending;
};

}  // namespace cave
