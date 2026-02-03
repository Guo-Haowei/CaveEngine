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

    virtual int GetPriority() const = 0;
    virtual void OnEvents(const InputFrame& p_input) = 0;
    virtual DebugId GetDebugId() = 0;
};

}  // namespace cave