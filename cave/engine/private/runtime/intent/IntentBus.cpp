#include <algorithm>

#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/intent/IntentBus.h"

#if USING(DEBUG_BUILD)
#include "cave/core/diagnostics/CommandRegistry.h"
#include "cave/core/diagnostics/ILogSink.h"
#endif

// #define WANT_TRACE_INTENT USE_IF(USING(USE_LOG))
#define WANT_TRACE_INTENT NOT_IN_USE
#if USING(WANT_TRACE_INTENT)
#define TRACE_INTENT(...)       \
    do {                        \
        LOG_TRACE(__VA_ARGS__); \
    } while (0)
#else
#define TRACE_INTENT(...) ((void)0)
#endif

namespace cave {

IntentBus::IntentBus() = default;

bool IntentBus::addHandler(IntentTypeId type_id, IIntentHandler* handler) {
    DEV_ASSERT(handler);

    auto [it, inserted] = m_handlers.try_emplace(type_id);
    if (!inserted) {
        if (it->first != type_id) {
            LOG_FATAL(LogChannel::Intent, "handler hash collision");
            return false;
        }
    }

    std::vector<IIntentHandler*>& handlers = it->second;

    auto it2 = std::find(handlers.begin(), handlers.end(), handler);
    if (it2 != handlers.end()) {
        LOG_ERROR(LogChannel::Intent, "handler '{}' already registered", handler->debugId().type);
        return false;
    }

    handlers.push_back(handler);
    [[maybe_unused]] const DebugId id = handler->debugId();
    TRACE_INTENT("Bind {}#{} -> {}", id.type, id.uid, type_id.debugName());
    return true;
}

bool IntentBus::removeHandler(IntentTypeId type_id, IIntentHandler* handler) {
    auto it = m_handlers.find(type_id);
    if (it == m_handlers.end()) {
        return false;
    }
    std::vector<IIntentHandler*>& handlers = it->second;
    auto it2 = std::remove(handlers.begin(), handlers.end(), handler);
    if (it2 == handlers.end()) {
        return false;
    }
    handlers.erase(it2, handlers.end());

    [[maybe_unused]] const DebugId id = handler->debugId();
    TRACE_INTENT("Unbind {}#{} -> {}", id.type, id.uid, type_id.debugName());
    return true;
}

void IntentBus::flush() {
    if (m_pending.empty()) {
        return;
    }

    std::vector<std::unique_ptr<Intent>> processing;
    std::swap(processing, m_pending);

    for (auto& intent : processing) {
        dispatchOne(*intent);
    }
}

void IntentBus::dispatchOne(Intent& intent) {
    auto it = m_handlers.find(intent.typeId());
    if (it == m_handlers.end()) {
        LOG_WARN(LogChannel::Intent, "IntentBus::DispatchOne: no handlers found for intent '{}'", intent.debugName());
        return;
    }

    for (IIntentHandler* handler : it->second) {
        if (DEV_VERIFY(handler)) {
            if (!handler->handleIntent(intent)) [[unlikely]] {
                LOG_ERROR(LogChannel::Intent,
                          "IntentBus: handler '{}' cant handle '{}'",
                          handler->debugId().type,
                          intent.debugName());
                continue;
            }

            TRACE_INTENT("{} {} [{}]",
                         intent.debugName(),
                         intent.debugString(),
                         handler->debugId().type);
        }
    }
}

#if USING(USE_COMMAND)
bool IntentBus::Cmd_dump(CommandContext& ctx, const CommandArgs&) {
    std::string msg;
    msg.reserve(512);
    msg.append("Registered Intent:");
    for (const auto& it : m_handlers) {
        msg.append(std::format("\n'{}' - ", it.first.debugName()));
        DEV_ASSERT(!it.second.empty());
        for (const IIntentHandler* handler : it.second) {
            const DebugId id = handler->debugId();
            msg.append(std::format("{}#{},", id.type, id.uid));
        }
        msg.pop_back();
    }
    ctx.log.Info(LogChannel::Console, std::move(msg));
    return true;
}
#endif

}  // namespace cave
