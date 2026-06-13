#include "InputRouter.h"

namespace cave {

void InputRouter::addConsumer(IInputConsumer* consumer) {
    DEV_ASSERT(consumer);
    if (!consumer) return;

    auto it = std::ranges::find(consumers_, consumer);
    if (it != consumers_.end()) return;

    consumers_.push_back(consumer);
    sortByPriority();

#if USING(USE_LOG)
    const DebugId id = consumer->debugId();
    LOG_TRACE(LogChannel::Input, "+{}#{}", id.type, id.uid);
#endif
}

void InputRouter::removeConsumer(IInputConsumer* consumer) {
    consumers_.erase(
        std::remove(consumers_.begin(), consumers_.end(), consumer),
        consumers_.end());

#if USING(USE_LOG)
    const DebugId id = consumer->debugId();
    LOG_TRACE(LogChannel::Input, "-{}#{}", id.type, id.uid);
#endif
}

void InputRouter::dispatch(const InputFrame& input) {
    for (auto* c : consumers_) {
        if (DEV_VERIFY(c)) {
            c->onEvents(input);
        }
    }
}

void InputRouter::sortByPriority() {
    std::sort(consumers_.begin(), consumers_.end(),
              [](const IInputConsumer* a, const IInputConsumer* b) {
                  return a->priority() > b->priority();
              });
}

}  // namespace cave
