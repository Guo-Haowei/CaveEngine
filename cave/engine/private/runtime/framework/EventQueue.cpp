#include "EventQueue.h"

namespace cave {

EventQueue::~EventQueue() {
#if USING(DEBUG_BUILD)
    if (!m_listeners.empty()) {
        LOG_ERROR(LogChannel::Core, "{} listeners not unregisted", m_listeners.size());
    }
#endif
}

void EventQueue::enqueueEvent(Owner<IEvent> event) {
    if (DEV_VERIFY(event)) {
        m_events.push(std::move(event));
    }
}

void EventQueue::dispatchEvent(const IEvent* event) {
    for (auto& listener : m_listeners) {
        listener->eventReceived(event);
    }
}

void EventQueue::flushEvents() {
    while (!m_events.empty()) {
        dispatchEvent(m_events.front().get());
        m_events.pop();
    }
}

void EventQueue::registerListener(EventListener* listener) {
    if (DEV_VERIFY(listener)) {
        m_listeners.push_back(listener);
    }
}

void EventQueue::unregisterListener(EventListener* listener) {
    auto it = std::find(m_listeners.begin(), m_listeners.end(), listener);
    if (DEV_VERIFY(it != m_listeners.end())) {
        m_listeners.erase(it);
    }
}

}  // namespace cave
