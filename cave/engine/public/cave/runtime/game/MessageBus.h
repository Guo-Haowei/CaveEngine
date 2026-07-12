// =============================================================================
// File: cave/runtime/game/MessageBus.h
// =============================================================================
#pragma once
#include <functional>

#include "cave/runtime/game/Message.h"

namespace cave {

using ListenerId = uint64_t;

class MessageBus {
public:
    using Callback = std::function<void(const Message&)>;

    ListenerId listen(StringId id, Callback&& callback);

    void disconnect(ListenerId listener_id);

    void emit(StringId id, ecs::Entity sender, Variant payload = {});

private:
    struct Listener {
        ListenerId id;
        Callback callback;
    };

	HashMap<StringId, Vector<Listener>> m_registration;
    ListenerId m_next_id = 0;
};

}  // namespace cave
