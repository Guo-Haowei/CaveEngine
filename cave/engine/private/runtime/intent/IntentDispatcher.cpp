#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/intent/IntentDispatcher.h"

#if USING(DEBUG_BUILD)
#include "cave/core/diagnostics/CommandRegistry.h"
#include "cave/core/diagnostics/ILogger.h"
#endif

#include <algorithm>

#if 1
#define DEBUG_PRINT(...) LOG_VERBOSE(__VA_ARGS__)
#else
#define DEBUG_PRINT(...) ((void)0)
#endif

namespace cave {

auto IntentDispatcher::InitializeImpl() -> Result<void> {
#if USING(DEBUG_BUILD)
    CommandRegistry& reg = m_app->CommandRegistry();
    reg.Register({
        .name = "intent.dump",
        .help = "List all registered intent handlers.",
        .usage = "intent.dump",
        .fn = [this](CommandContext& p_ctx, const CommandArgs& p_args) {
            IntentDispatcherDump_Cmd(p_ctx, p_args);
            return true;
        },
    });
#endif

    return Result<void>();
}

void IntentDispatcher::FinalizeImpl() {
}

bool IntentDispatcher::AddHandler(IntentTypeId p_intent_id, IIntentHandler* p_handler) {
    DEV_ASSERT(p_handler);

    auto [it, inserted] = m_handlers.try_emplace(p_intent_id);
    if (!inserted) {
        if (it->first != p_intent_id) {
            LOG_FATAL("IntentDispatcher::AddHandler: hash collision");
            return false;
        }
    }

    std::vector<IIntentHandler*>& handlers = it->second;

    auto it2 = std::find(handlers.begin(), handlers.end(), p_handler);
    if (it2 != handlers.end()) {
        LOG_ERROR("IntentDispatcher::AddHandler: handler '{}' already registered", p_handler->GetDebugId().type);
        return false;
    }

    handlers.push_back(p_handler);
    return true;
}

bool IntentDispatcher::RemoveHandler(IntentTypeId p_intent_id, IIntentHandler* p_handler) {
    auto it = m_handlers.find(p_intent_id);
    if (it == m_handlers.end()) return false;
    std::vector<IIntentHandler*>& handlers = it->second;
    auto it2 = std::remove(handlers.begin(), handlers.end(), p_handler);
    if (it2 == handlers.end()) return false;
    handlers.erase(it2, handlers.end());
    return true;
}

void IntentDispatcher::Flush() {
    for (auto& intent : m_intents) {
        DEBUG_PRINT("IntentDispatcher::Flush: handle intent '{}'", intent->GetDebugName());
        DispatchOne(*intent);
    }
    m_intents.clear();
}

void IntentDispatcher::DispatchOne(Intent& p_intent) {
    auto it = m_handlers.find(p_intent.GetTypeId());
    if (it == m_handlers.end()) {
        LOG_WARN("IntentDispatcher::DispatchOne: no handlers found for intent '{}'", p_intent.GetDebugName());
        return;
    }

    for (IIntentHandler* handler : it->second) {
        if (DEV_VERIFY(handler)) {
            if (!handler->HandleIntent(p_intent)) [[unlikely]] {
                LOG_ERROR("IntentDispatcher: handler '{}' cant handle '{}'",
                          handler->GetDebugId().type,
                          p_intent.GetDebugName());
            }
        }
    }
}

void IntentDispatcher::IntentDispatcherDump_Cmd(CommandContext& p_ctx, const CommandArgs&) {
#if USING(DEBUG_BUILD)
    std::string msg;
    msg.reserve(512);
    msg.append("Registered Intent:\n");
    for (const auto& it : m_handlers) {
        msg.append(std::format("'{}' - ", it.first.DebugName()));
        for (const IIntentHandler* handler : it.second) {
            msg.append(std::format("{},", handler->GetDebugId().type));
        }
        msg[msg.size() - 1] = '\n';  // replace ',' with new line
    }
    p_ctx.logger.Print(LogLevel::LOG_LEVEL_VERBOSE, msg);
#else
    unused(p_ctx);
#endif
}

}  // namespace cave
