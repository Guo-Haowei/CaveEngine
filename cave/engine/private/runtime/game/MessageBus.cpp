#include "cave/runtime/game/MessageBus.h"

namespace cave {

ListenerId MessageBus::listen(StringId id, Callback&& callback) {
    const ListenerId listener_id = ++m_next_id;

    m_registration[id].push_back(Listener{
        .id = listener_id,
        .callback = std::move(callback),
    });

    return listener_id;
}

void MessageBus::disconnect(ListenerId listener_id) {
    for (auto& [id, listener] : m_registration) {
        std::erase_if(listener, [listener_id](const Listener& listener) {
            return listener.id == listener_id;
        });
    }
}

void MessageBus::emit(StringId id, ecs::Entity sender, Variant payload) {
    auto it = m_registration.find(id);
    if (it == m_registration.end()) {
        return;
    }

    Message message{
        .id = id,
        .sender = sender,
        .payload = payload,
    };

    const auto listeners = it->second;

    for (const Listener& listener : listeners) {
        listener.callback(message);
    }
}

}  // namespace cave
