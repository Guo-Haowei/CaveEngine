// =============================================================================
// File: public/cave/runtime/input/IInputConsumer.h
// =============================================================================
#pragma once
#include "cave/core/ids/DebugId.h"
#include "cave/runtime/input/InputTypes.h"

namespace cave {

class KeyState;

struct InputFrame {
    float dt;
    std::span<const InputEvent> events;
    KeyState& keystate;
};

class IInputConsumer {
public:
    virtual ~IInputConsumer() = default;

    virtual void onEvents(const InputFrame& input) = 0;
    virtual int priority() const = 0;
    virtual DebugId debugId() const = 0;
};

}  // namespace cave