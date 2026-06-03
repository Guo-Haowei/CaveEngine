#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/intent/IntentDispatcher.h"

#if USING(DEBUG_BUILD)
#include "cave/core/diagnostics/CommandRegistry.h"
#include "cave/core/diagnostics/ILogSink.h"
#endif

#include <algorithm>

#if 1
#define DEBUG_PRINT(...) LOG_TRACE(__VA_ARGS__)
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
            LOG_FATAL(LogChannel::Intent, "handler hash collision");
            return false;
        }
    }

    std::vector<IIntentHandler*>& handlers = it->second;

    auto it2 = std::find(handlers.begin(), handlers.end(), p_handler);
    if (it2 != handlers.end()) {
        LOG_ERROR(LogChannel::Intent, "handler '{}' already registered", p_handler->GetDebugId().type);
        return false;
    }

    handlers.push_back(p_handler);
#if USING(USE_LOG)
    const DebugId id = p_handler->GetDebugId();
    LOG_TRACE(LogChannel::Intent, "Bind {}#{} -> {}", id.type, id.uid, p_intent_id.DebugName());
#endif
    return true;
}

bool IntentDispatcher::RemoveHandler(IntentTypeId p_intent_id, IIntentHandler* p_handler) {
    auto it = m_handlers.find(p_intent_id);
    if (it == m_handlers.end()) {
        return false;
    }
    std::vector<IIntentHandler*>& handlers = it->second;
    auto it2 = std::remove(handlers.begin(), handlers.end(), p_handler);
    if (it2 == handlers.end()) {
        return false;
    }
    handlers.erase(it2, handlers.end());

#if USING(USE_LOG)
    const DebugId id = p_handler->GetDebugId();
    LOG_TRACE(LogChannel::Intent, "Unbind {}#{} -> {}", id.type, id.uid, p_intent_id.DebugName());
#endif
    return true;
}

void IntentDispatcher::Flush() {
    for (auto& intent : m_intents) {
        DEBUG_PRINT(LogChannel::Intent, "{} {}", intent->GetDebugName(), intent->DebugString());
        DispatchOne(*intent);
    }
    m_intents.clear();
}

void IntentDispatcher::DispatchOne(Intent& p_intent) {
    auto it = m_handlers.find(p_intent.GetTypeId());
    if (it == m_handlers.end()) {
        LOG_WARN(LogChannel::Intent, "IntentDispatcher::DispatchOne: no handlers found for intent '{}'", p_intent.GetDebugName());
        return;
    }

    for (IIntentHandler* handler : it->second) {
        if (DEV_VERIFY(handler)) {
            if (!handler->HandleIntent(p_intent)) [[unlikely]] {
                LOG_ERROR(LogChannel::Intent,
                          "IntentDispatcher: handler '{}' cant handle '{}'",
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
    msg.append("Registered Intent:");
    for (const auto& it : m_handlers) {
        msg.append(std::format("\n'{}' - ", it.first.DebugName()));
        DEV_ASSERT(!it.second.empty());
        for (const IIntentHandler* handler : it.second) {
            const DebugId id = handler->GetDebugId();
            msg.append(std::format("{}#{},", id.type, id.uid));
        }
        msg.pop_back();
    }
    p_ctx.log.Info(LogChannel::Console, std::move(msg));
#else
    unused(p_ctx);
#endif
}

}  // namespace cave
